package kts.complex
import kotlinx.serialization.Serializable
import kotlinx.serialization.protobuf.*
import google.protobuf.*
@Serializable enum class EnumerationName (
  val value: Int,
) {
  ENUM_VALUE_NAME(0),
}
@Serializable data class MessageName(
  @ProtoNumber(1) val messageFieldName: Int = 0,
  @ProtoOneOf val oneOfFieldName: IOneOfFieldName = IOneOfFieldName.LinuxTimestamp(),
) {
  @Serializable sealed interface IOneOfFieldName {
    @Serializable data class LinuxTimestamp (
      @ProtoNumber(2) val value: Long = 0L,
    ) : IOneOfFieldName
    @Serializable data class StructTimestamp (
      @ProtoNumber(3) val value: Timestamp = Timestamp(),
    ) : IOneOfFieldName
  }
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as MessageName
    return true &&
      messageFieldName.equals(other.messageFieldName) &&
      oneOfFieldName.equals(other.oneOfFieldName)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + messageFieldName.hashCode()
    result = 31 * result + oneOfFieldName.hashCode()
    return result
  }
}
