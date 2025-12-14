package kts.complex
import kotlinx.serialization.Serializable
import kotlinx.serialization.protobuf.*
@Serializable enum class Enumeration (
  val value: Int,
) {
  UNSPECIFIED(0),
  VALUE_ONE(1),
  VALUE_TWO(2),
}
@Serializable data class Collections(
  @ProtoNumber(1) val repeatedField: List<Int> = emptyList(),
  @ProtoNumber(2) val mapField: Map<Int, String> = emptyMap(),
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Collections
    return true &&
      repeatedField.equals(other.repeatedField) &&
      mapField.equals(other.mapField)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + repeatedField.hashCode()
    result = 31 * result + mapField.hashCode()
    return result
  }
}
@Serializable data class Message(
  @ProtoNumber(1) val nested: NestedMessage = NestedMessage(),
  @ProtoOneOf val message: IMessage = IMessage.PlainText(),
) {
  @Serializable class NestedMessage(
  ) {
    override fun equals(other: Any?): Boolean {
      if (this === other) return true
      if (javaClass != other?.javaClass) return false
      other as NestedMessage
      return true
    }
    override fun hashCode(): Int {
      var result = 0
      return result
    }
  }
  @Serializable sealed interface IMessage {
    @Serializable data class PlainText (
      @ProtoNumber(2) val value: String = "",
    ) : IMessage
    @Serializable data class Encoded (
      @ProtoNumber(3) val value: ByteArray = ByteArray(0),
    ) : IMessage
  }
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Message
    return true &&
      nested.equals(other.nested) &&
      message.equals(other.message)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + nested.hashCode()
    result = 31 * result + message.hashCode()
    return result
  }
}
