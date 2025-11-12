#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>

using google::protobuf::Descriptor;
using google::protobuf::Edition;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::FieldDescriptorProto;
using google::protobuf::FileDescriptor;
using google::protobuf::OneofDescriptor;
using google::protobuf::SourceLocation;
using google::protobuf::compiler::CodeGenerator;
using google::protobuf::compiler::GeneratorContext;
using google::protobuf::compiler::PluginMain;
using google::protobuf::io::Printer;
using google::protobuf::io::ZeroCopyOutputStream;

class KtsCodeGenerator : public CodeGenerator {
    bool Generate(const FileDescriptor *fileDescriptor, const std::string &parameter,
                  GeneratorContext *generatorContext, std::string *error) const override {
        print(fileDescriptor, generatorContext);
        return true;
    }

    static void print(const FileDescriptor *descriptor, GeneratorContext *generatorContext) {
        const std::string filename(std::string(descriptor->name()) + ".kt");
        std::unique_ptr<ZeroCopyOutputStream> stream(generatorContext->Open(filename));
        Printer printer(stream.get());

        printer.PrintRaw("@file:OptIn(kotlinx.serialization.ExperimentalSerializationApi::class)\n");

        if (!descriptor->package().empty())
            printer.Print("package $name$\n", "name", descriptor->package());

        printer.PrintRaw(
            "import kotlinx.serialization.Serializable\n"
            "import kotlinx.serialization.protobuf.*\n"
        );

        for (int i(0); i < descriptor->dependency_count(); ++i)
            print(printer, descriptor->dependency(i));
        for (int i(0); i < descriptor->message_type_count(); ++i)
            print(printer, descriptor->message_type(i));
    }

    static void print(Printer &printer, const FileDescriptor *descriptor) {
        if (!descriptor->package().empty())
            printer.Print("import $package$.*\n", "package", descriptor->package());
    }

    static void print(Printer &printer, const Descriptor *descriptor) {
        printer.Print("@Serializable\nclass $name$(\n",
                      "name", descriptor->name());

        printer.Indent();
        std::vector<const OneofDescriptor *> oneOfDescriptors;
        for (int i(0); i < descriptor->field_count(); ++i) {
            const FieldDescriptor *fieldDescriptor(descriptor->field(i));
            if (const OneofDescriptor *oneOfDescriptor(fieldDescriptor->containing_oneof()); oneOfDescriptor) {
                printer.Print(
                    "@ProtoOneOf val $name$: $className$ = $className$.$field$(),\n",
                    "name", oneOfDescriptor->name(), "className", getOneOfName(oneOfDescriptor->name()),
                    "field", toPascalCase(fieldDescriptor->camelcase_name())
                );
                oneOfDescriptors.push_back(oneOfDescriptor);
                i += oneOfDescriptor->field_count() - 1;
            } else {
                print(printer, fieldDescriptor);
            }
        }
        printer.Outdent();

        printer.Print(") {\n");
        printer.Indent();
        for (int i(0); i < descriptor->enum_type_count(); ++i)
            print(printer, descriptor->enum_type(i));
        for (int i(0); i < descriptor->nested_type_count(); ++i)
            print(printer, descriptor->nested_type(i));
        for (int i(0); i < descriptor->field_count(); ++i) {
            if (const OneofDescriptor *oneOfDescriptor(descriptor->field(i)->containing_oneof()); oneOfDescriptor) {
                print(printer, oneOfDescriptor);
                i += oneOfDescriptor->field_count() - 1;
            }
        }
        printer.Outdent();
        printer.Print("}\n");
    }

    static void print(Printer &printer, const EnumDescriptor *descriptor) {
        printer.Print("@Serializable enum class $name$ (\n",
                      "name", descriptor->name());
        printer.Indent();
        printer.Print("val value: Int,\n");
        printer.Outdent();
        printer.Print(") {\n");

        printer.Indent();
        for (int i(0); i < descriptor->value_count(); ++i)
            print(printer, descriptor->value(i));
        printer.Outdent();

        printer.Print("}\n");
    }

    static void print(Printer &printer, const EnumValueDescriptor *descriptor) {
        printer.Print(descriptor->name());
        printer.Print("($number$),\n", "number", std::to_string(descriptor->number()));
    }

    static void print(Printer &printer, const OneofDescriptor *descriptor) {
        const std::string typeName(getOneOfName(descriptor->name()));
        printer.Print("@Serializable sealed interface $name$ {\n", "name", typeName);
        printer.Indent();
        for (int i(0); i < descriptor->field_count(); ++i) {
            const FieldDescriptor *fieldDescriptor(descriptor->field(i));
            printer.Print("@Serializable data class $name$ (\n", "name",
                          toPascalCase(fieldDescriptor->camelcase_name()));
            printer.Indent();
            print(printer, fieldDescriptor, "value");
            printer.Outdent();
            printer.Print(") : $name$\n", "name", typeName);
        }
        printer.Outdent();
        printer.Print("}\n");
    }

    static void print(Printer &printer, const FieldDescriptor *descriptor) {
        print(printer, descriptor, descriptor->camelcase_name());
    }

    static void print(Printer &printer, const FieldDescriptor *descriptor, const std::string_view fieldName) {
        printer.Print("@ProtoNumber($number$) val $name$: ", "name", fieldName, "number",
                      std::to_string(descriptor->number()));

        printType(printer, descriptor);
        printer.PrintRaw(" = ");
        printTypeDefaultValue(printer, descriptor);
        printer.Print(",\n");
    }

    static void printType(Printer &printer, const FieldDescriptor *descriptor) {
        if (descriptor->is_repeated())
            printer.PrintRaw("List<");
        switch (descriptor->type()) {
            case FieldDescriptor::Type::TYPE_STRING:
                printer.PrintRaw("String");
                break;
            case FieldDescriptor::Type::TYPE_MESSAGE:
                printer.PrintRaw(descriptor->message_type()->name());
                break;
            case FieldDescriptor::Type::TYPE_BYTES:
                printer.PrintRaw("ByteArray");
                break;
            case FieldDescriptor::Type::TYPE_UINT32:
                printer.PrintRaw("UInt");
                break;
            case FieldDescriptor::Type::TYPE_ENUM:
                printer.PrintRaw(descriptor->enum_type()->name());
                break;
            default:
                throw std::runtime_error("Field type " + std::to_string(descriptor->type()));
        }
        if (descriptor->is_repeated())
            printer.PrintRaw(">");
    }

    static void printTypeDefaultValue(Printer &printer, const FieldDescriptor *descriptor) {
        if (descriptor->is_repeated()) {
            printer.PrintRaw("emptyList()");
            return;
        }
        switch (descriptor->type()) {
            case FieldDescriptor::Type::TYPE_STRING:
                printer.PrintRaw("\"\"");
                break;
            case FieldDescriptor::Type::TYPE_MESSAGE:
                printer.PrintRaw(descriptor->message_type()->name());
                printer.PrintRaw("()");
                break;
            case FieldDescriptor::Type::TYPE_BYTES:
                printer.PrintRaw("byteArrayOf()");
                break;
            case FieldDescriptor::Type::TYPE_UINT32:
                printer.PrintRaw("0u");
                break;
            case FieldDescriptor::Type::TYPE_ENUM:
                printer.PrintRaw(descriptor->enum_type()->name());
                printer.PrintRaw(".entries.first()");
                break;
            default:
                throw std::runtime_error("Field type " + std::to_string(descriptor->type()));
        }
    }

    static std::string getOneOfName(const std::string_view &value) {
        return toPascalCase(value) + "OneOf";
    }

    static std::string toPascalCase(const std::string_view &value) {
        if (value.find('_') == std::string_view::npos) {
            std::string result(value);
            result[0] = std::toupper(result[0]);
            return result;
        }
        std::string result;
        result.reserve(value.size());
        bool capitalize = true;
        for (char c: value) {
            if (c == '_') {
                capitalize = true;
            } else if (capitalize) {
                result.push_back(std::toupper(c));
                capitalize = false;
            } else {
                result.push_back(c);
            }
        }
        return result;
    }

    [[nodiscard]]
    uint64_t GetSupportedFeatures() const override {
        return Feature::FEATURE_SUPPORTS_EDITIONS;
    }

    [[nodiscard]]
    Edition GetMinimumEdition() const override {
        return Edition::EDITION_2024;
    }

    [[nodiscard]]
    Edition GetMaximumEdition() const override {
        return Edition::EDITION_2024;
    }
};

int main(int argc, char **argv) {
    const KtsCodeGenerator ktsCodeGenerator;
    return PluginMain(argc, argv, &ktsCodeGenerator);
}
