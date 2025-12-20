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
@Serializable data class Message(
  @ProtoOneOf val message: IMessage = IMessage.PlainText(),
) {
  @Serializable sealed interface IMessage {
    @Serializable data class PlainText (
      @ProtoNumber(1) val value: String = "",
    ) : IMessage
    @Serializable data class Encoded (
      @ProtoNumber(2) val value: ByteArray = byteArrayOf(),
    ) : IMessage
  }
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Message
    return true &&
      message.equals(other.message)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + message.hashCode()
    return result
  }
}
