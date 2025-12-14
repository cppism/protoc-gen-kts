#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>

using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace google::protobuf::io;

class KtsException : public std::exception {
    const std::string _message;

public:
    explicit KtsException(std::string message) : _message(std::move(message)) {
    }

    [[nodiscard]]
    const char *what() const noexcept override {
        return _message.c_str();
    }
};

template<typename TContainer, typename TItem>
class Iterator {
    using GetItemFunc = TItem *(TContainer::*)(int) const;

    const TContainer *_container;
    const GetItemFunc _getItem;
    const int _count;
    int _index;

public:
    Iterator(const TContainer *container, const GetItemFunc getItem, const int count) : _container(container),
        _getItem(getItem), _count(count),
        _index(0) {
    }

    TItem *operator*() const {
        return (_container->*_getItem)(_index);
    }

    Iterator &operator++() {
        ++_index;
        return *this;
    }

    bool operator!=(const std::default_sentinel_t & /*unused*/) const {
        return _index < _count;
    }
};

template<typename TContainer, typename TItem>
class Range {
    using GetItemFunc = TItem *(TContainer::*)(int) const;

    const TContainer *_container;
    const GetItemFunc _getItem;
    const int _count;

public:
    Range(const TContainer *container, const GetItemFunc getItem,
          int (TContainer::*getCount)() const) : _container(container),
                                                 _getItem(getItem),
                                                 _count((container->*getCount)()) {
    }

    [[nodiscard]]
    Iterator<TContainer, TItem> begin() const noexcept {
        return Iterator(_container, _getItem, _count);
    }

    std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }
};

class KtsCodeGenerator : public CodeGenerator {
public:
    uint64_t GetSupportedFeatures() const override {
        return Feature::FEATURE_SUPPORTS_EDITIONS;
    }

    Edition GetMinimumEdition() const override {
        return Edition::EDITION_2024;
    }

    Edition GetMaximumEdition() const override {
        return Edition::EDITION_2024;
    }

    bool Generate(const FileDescriptor *fileDescriptor, const std::string & /*parameter*/,
                  GeneratorContext *generatorContext, std::string *error) const override {
        try {
            print(fileDescriptor, generatorContext);
        } catch (const KtsException &ex) {
            error->assign(ex.what());
            return false;
        }
        return true;
    }

private:
    template<typename TContainer, typename TItem>
    static void print(
        Printer &printer,
        const TContainer *descriptor,
        const std::function<void(Printer &, const TItem *)> &delegate,
        const TItem *(TContainer::*getItem)(int) const,
        int (TContainer::*getCount)() const,
        const std::function<bool(const TItem *)> &predicate = [](const TItem *) -> bool { return true; }
    ) {
        for (const TItem *item: Range(descriptor, getItem, getCount))
            if (predicate(item))
                delegate(printer, item);
    }

    static void print(const FileDescriptor *descriptor, GeneratorContext *generatorContext) {
        const std::string filename(std::string(descriptor->name()) + ".kt");
        const std::unique_ptr<ZeroCopyOutputStream> stream(generatorContext->Open(filename));
        Printer printer(stream.get());

        if (!descriptor->package().empty())
            printer.Print("package $package$\n", "package", descriptor->package());

        printer.PrintRaw(
            "import kotlinx.serialization.Serializable\n"
            "import kotlinx.serialization.protobuf.*\n"
        );

        print<FileDescriptor, FileDescriptor>(
            printer, descriptor, printImport, &FileDescriptor::dependency, &FileDescriptor::dependency_count
        );
        print<FileDescriptor, EnumDescriptor>(
            printer, descriptor, printEnum, &FileDescriptor::enum_type, &FileDescriptor::enum_type_count
        );
        print<FileDescriptor, Descriptor>(
            printer, descriptor, printMessage, &FileDescriptor::message_type, &FileDescriptor::message_type_count
        );
        print<FileDescriptor, FieldDescriptor>(
            printer, descriptor, printExtension, &FileDescriptor::extension, &FileDescriptor::extension_count
        );
    }

    static void printImport(Printer &printer, const FileDescriptor *descriptor) {
        if (!descriptor->package().empty())
            printer.Print("import $package$.*\n", "package", descriptor->package());
    }

    static void printEnum(Printer &printer, const EnumDescriptor *descriptor) {
        printer.Print("@Serializable enum class $class$ (\n",
                      "class", descriptor->name());
        printer.Indent();
        printer.Print("val value: Int,\n");
        printer.Outdent();
        printer.Print(") {\n");

        printer.Indent();
        print<EnumDescriptor, EnumValueDescriptor>(
            printer, descriptor, printEnumValue, &EnumDescriptor::value, &EnumDescriptor::value_count
        );
        printer.Outdent();

        printer.Print("}\n");
    }

    static void printEnumValue(Printer &printer, const EnumValueDescriptor *descriptor) {
        printer.PrintRaw(descriptor->name());
        printer.Print("($number$),\n", "number", std::to_string(descriptor->number()));
    }

    static void printMessage(Printer &printer, const Descriptor *descriptor) {
        printer.PrintRaw("@Serializable ");
        printer.PrintRaw(descriptor->field_count() == 0 ? "class " : "data class ");
        printer.Print("$class$(\n", "class", descriptor->name());

        printer.Indent();
        std::vector<const OneofDescriptor *> oneOfDescriptors;
        for (int i(0); i < descriptor->field_count(); ++i) {
            const FieldDescriptor *fieldDescriptor(descriptor->field(i));
            if (const OneofDescriptor *oneOfDescriptor(fieldDescriptor->containing_oneof()); oneOfDescriptor) {
                printer.Print(
                    "@ProtoOneOf val $name$: $class$ = $class$.$field$(),\n",
                    "name", toCamelCase(oneOfDescriptor->name()), "class", getOneOfName(oneOfDescriptor->name()),
                    "field", toPascalCase(fieldDescriptor->camelcase_name())
                );
                oneOfDescriptors.push_back(oneOfDescriptor);
                i += oneOfDescriptor->field_count() - 1;
            } else {
                printField(printer, fieldDescriptor);
            }
        }
        printer.Outdent();

        printer.Print(") {\n");
        printer.Indent();
        print<Descriptor, EnumDescriptor>(
            printer, descriptor, printEnum, &Descriptor::enum_type, &Descriptor::enum_type_count
        );
        print<Descriptor, Descriptor>(
            printer, descriptor, printMessage, &Descriptor::nested_type, &Descriptor::nested_type_count,
            [](const Descriptor *item) -> bool { return item->map_key() == nullptr; }
        );
        print<Descriptor, FieldDescriptor>(
            printer, descriptor, printExtension, &Descriptor::extension, &Descriptor::extension_count
        );
        for (int i(0); i < descriptor->field_count(); ++i)
            if (const OneofDescriptor *oneOfDescriptor(descriptor->field(i)->containing_oneof()); oneOfDescriptor)
                i += printOneOfType(printer, oneOfDescriptor) - 1;

        printEqualsFunction(printer, descriptor);
        printHashCodeFunction(printer, descriptor);
        printer.Outdent();
        printer.Print("}\n");
    }

    static void printEqualsFunction(Printer &printer, const Descriptor *descriptor) {
        printer.Print("override fun equals(other: Any?): Boolean {\n");
        printer.Indent();
        printer.Print(
            "if (this === other) return true\n"
            "if (javaClass != other?.javaClass) return false\n"
            "other as $name$\n", "name", descriptor->name()
        );
        printer.PrintRaw("return true");
        printer.Indent();
        printFunctionBody(
            printer, descriptor, " &&\n$field$.$func$(other.$field$)",
            [](const FieldDescriptor::Type fieldType) -> std::string_view {
                return fieldType == FieldDescriptor::TYPE_BYTES ? "contentEquals" : "equals";
            }
        );
        printer.Outdent();
        printer.Print("\n");
        printer.Outdent();
        printer.Print("}\n");
    }

    static void printHashCodeFunction(Printer &printer, const Descriptor *descriptor) {
        printer.Print("override fun hashCode(): Int {\n");
        printer.Indent();
        printer.Print("var result = 0\n");
        printFunctionBody(
            printer, descriptor, "result = 31 * result + $field$.$func$()\n",
            [](const FieldDescriptor::Type fieldType) -> std::string_view {
                return fieldType == FieldDescriptor::TYPE_BYTES ? "contentHashCode" : "hashCode";
            }
        );
        printer.Print("return result\n");
        printer.Outdent();
        printer.Print("}\n");
    }

    static void printFunctionBody(
        Printer &printer, const Descriptor *descriptor, const std::string_view expressionTemplate,
        const std::function<std::string_view(FieldDescriptor::Type)> &getFunctionName
    ) {
        for (int i(0); i < descriptor->field_count(); ++i) {
            const FieldDescriptor *fieldDescriptor(descriptor->field(i));
            std::string fieldName(fieldDescriptor->camelcase_name());
            std::string_view functionName(getFunctionName(fieldDescriptor->type()));
            if (const OneofDescriptor *oneOfDescriptor(fieldDescriptor->containing_oneof()); oneOfDescriptor) {
                fieldName = toCamelCase(oneOfDescriptor->name());
                functionName = getFunctionName(FieldDescriptor::TYPE_MESSAGE);
                i += oneOfDescriptor->field_count() - 1;
            }
            printer.Print(expressionTemplate, "field", fieldName, "func", functionName);
        }
    }

    static int printOneOfType(Printer &printer, const OneofDescriptor *descriptor) {
        const std::string typeName(getOneOfName(descriptor->name()));
        printer.Print("@Serializable sealed interface $name$ {\n", "name", typeName);
        printer.Indent();
        for (int i(0); i < descriptor->field_count(); ++i) {
            const FieldDescriptor *fieldDescriptor(descriptor->field(i));
            printer.Print("@Serializable data class $name$ (\n", "name",
                          toPascalCase(fieldDescriptor->camelcase_name()));
            printer.Indent();
            printField(printer, fieldDescriptor, "value");
            printer.Outdent();
            printer.Print(") : $type$\n", "type", typeName);
        }
        printer.Outdent();
        printer.Print("}\n");
        return descriptor->field_count();
    }

    static void printExtension(Printer &printer, const FieldDescriptor *descriptor) {
        printer.Print(
            "@Deprecated(\""
            "Kotlin doesn't support neither partial classes nor extension properties serialization"
            "\")\n"
        );
        printer.Print(
            "var $type$.$property$: ",
            "type", descriptor->containing_type()->full_name(),
            "property", descriptor->camelcase_name()
        );
        printFieldType(printer, descriptor);
        printer.Print("\n");
        printer.Indent();
        printer.PrintRaw("get() = ");
        printDefaultValue(printer, descriptor);
        printer.Print("\nset(_) {}\n");
        printer.Outdent();
    }

    static void printField(Printer &printer, const FieldDescriptor *descriptor) {
        printField(printer, descriptor, descriptor->camelcase_name());
    }

    static void printField(Printer &printer, const FieldDescriptor *descriptor, const std::string_view fieldName) {
        printer.Print("@ProtoNumber($number$) ", "number", std::to_string(descriptor->number()));
        printTypeAnnotation(printer, descriptor);
        printer.Print("val $name$: ", "name", fieldName);
        printFieldType(printer, descriptor);
        printer.PrintRaw(" = ");
        printDefaultValue(printer, descriptor);
        printer.Print(",\n");
    }

    static void printFieldType(Printer &printer, const FieldDescriptor *descriptor) {
        if (descriptor->is_repeated())
            if (descriptor->is_map())
                printMapType(printer, descriptor);
            else
                printListType(printer, descriptor);
        else
            printRawType(printer, descriptor);
    }

    static void printListType(Printer &printer, const FieldDescriptor *descriptor) {
        printer.PrintRaw("List<");
        printRawType(printer, descriptor);
        printer.PrintRaw(">");
    }

    static void printMapType(Printer &printer, const FieldDescriptor *descriptor) {
        const Descriptor *mapDescriptor(descriptor->message_type());
        printer.PrintRaw("Map<");
        printFieldType(printer, mapDescriptor->map_key());
        printer.PrintRaw(", ");
        printFieldType(printer, mapDescriptor->map_value());
        printer.PrintRaw(">");
    }

    static void printRawType(Printer &printer, const FieldDescriptor *descriptor) {
        printer.PrintRaw(getTypeName(descriptor));
    }

    static std::string_view getTypeName(const FieldDescriptor *descriptor) {
        switch (descriptor->type()) {
            case FieldDescriptor::Type::TYPE_DOUBLE:
                return "Double";
            case FieldDescriptor::Type::TYPE_FLOAT:
                return "Float";
            case FieldDescriptor::Type::TYPE_INT64:
            case FieldDescriptor::Type::TYPE_SFIXED64:
            case FieldDescriptor::Type::TYPE_SINT64:
                return "Long";
            case FieldDescriptor::Type::TYPE_UINT64:
            case FieldDescriptor::Type::TYPE_FIXED64:
                return "ULong";
            case FieldDescriptor::Type::TYPE_INT32:
            case FieldDescriptor::Type::TYPE_SFIXED32:
            case FieldDescriptor::Type::TYPE_SINT32:
                return "Int";
            case FieldDescriptor::Type::TYPE_UINT32:
            case FieldDescriptor::Type::TYPE_FIXED32:
                return "UInt";
            case FieldDescriptor::Type::TYPE_BOOL:
                return "Boolean";
            case FieldDescriptor::Type::TYPE_STRING:
                return "String";
            case FieldDescriptor::Type::TYPE_MESSAGE:
                return descriptor->message_type()->name();
            case FieldDescriptor::Type::TYPE_BYTES:
                return "ByteArray";
            case FieldDescriptor::Type::TYPE_ENUM:
                return descriptor->enum_type()->name();
            default:
                throw KtsException("Field type " + std::to_string(descriptor->type()) + "is not supported");
        }
    }

    static void printTypeAnnotation(Printer &printer, const FieldDescriptor *descriptor) {
        switch (descriptor->type()) {
            case FieldDescriptor::Type::TYPE_FIXED32:
            case FieldDescriptor::Type::TYPE_FIXED64:
                printer.PrintRaw("@ProtoType(ProtoIntegerType.FIXED) ");
                break;
            case FieldDescriptor::Type::TYPE_SINT32:
            case FieldDescriptor::Type::TYPE_SINT64:
                printer.PrintRaw("@ProtoType(ProtoIntegerType.SIGNED) ");
                break;
            default: {
            }
        }
    }

    static void printDefaultValue(Printer &printer, const FieldDescriptor *descriptor) {
        if (descriptor->is_repeated()) {
            printer.PrintRaw("empty");
            printer.PrintRaw(descriptor->is_map() ? "Map" : "List");
            printer.PrintRaw("()");
            return;
        }
        switch (descriptor->type()) {
            case FieldDescriptor::Type::TYPE_DOUBLE:
                printer.PrintRaw("0.0");
                break;
            case FieldDescriptor::Type::TYPE_FLOAT:
                printer.PrintRaw("0F");
                break;
            case FieldDescriptor::Type::TYPE_INT64:
            case FieldDescriptor::Type::TYPE_SFIXED64:
            case FieldDescriptor::Type::TYPE_SINT64:
                printer.PrintRaw("0L");
                break;
            case FieldDescriptor::Type::TYPE_UINT64:
            case FieldDescriptor::Type::TYPE_FIXED64:
                printer.PrintRaw("0UL");
                break;
            case FieldDescriptor::Type::TYPE_INT32:
            case FieldDescriptor::Type::TYPE_SFIXED32:
            case FieldDescriptor::Type::TYPE_SINT32:
                printer.PrintRaw("0");
                break;
            case FieldDescriptor::Type::TYPE_UINT32:
            case FieldDescriptor::Type::TYPE_FIXED32:
                printer.PrintRaw("0U");
                break;
            case FieldDescriptor::Type::TYPE_BOOL:
                printer.PrintRaw("false");
                break;
            case FieldDescriptor::Type::TYPE_STRING:
                printer.PrintRaw("\"\"");
                break;
            case FieldDescriptor::Type::TYPE_MESSAGE:
                printer.PrintRaw(descriptor->message_type()->name());
                printer.PrintRaw("()");
                break;
            case FieldDescriptor::Type::TYPE_BYTES:
                printer.PrintRaw("ByteArray(0)");
                break;
            case FieldDescriptor::Type::TYPE_ENUM:
                printer.PrintRaw(descriptor->enum_type()->name());
                printer.PrintRaw(".entries.first()");
                break;
            case FieldDescriptor::Type::TYPE_GROUP:
                break;
        }
    }

    static std::string getOneOfName(const std::string_view &value) {
        return "I" + toPascalCase(value);
    }

    static std::string toCamelCase(const std::string_view &value) {
        return toUpperAfterUnderscores(value, false);
    }

    static std::string toPascalCase(const std::string_view &value) {
        return toUpperAfterUnderscores(value, true);
    }

    static std::string toUpperAfterUnderscores(const std::string_view &value, bool capitalize) {
        if (!value.contains('_')) {
            std::string result(value);
            if (capitalize)
                result[0] = static_cast<char>(std::toupper(result.at(0)));
            return result;
        }
        std::string result;
        result.reserve(value.size());
        for (const char c: value) {
            if (c == '_') {
                capitalize = true;
            } else if (capitalize) {
                capitalize = false;
                result.push_back(static_cast<char>(std::toupper(c)));
            } else {
                result.push_back(c);
            }
        }
        return result;
    }
};

int main(int argc, char **argv) {
    const KtsCodeGenerator ktsCodeGenerator;
    return PluginMain(argc, argv, &ktsCodeGenerator);
}
