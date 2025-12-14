package kts.package_name
import kotlinx.serialization.Serializable
import kotlinx.serialization.protobuf.*
@Serializable enum class EnumerationName (
  val value: Int,
) {
  ENUM_VALUE_NAME(0),
}
@Serializable data class MessageName(
  @ProtoNumber(1) val scalarFieldName: Int = 0,
  @ProtoOneOf val oneOfFieldName: IOneOfFieldName = IOneOfFieldName.IsoStringField(),
) {
  @Serializable sealed interface IOneOfFieldName {
    @Serializable data class IsoStringField (
      @ProtoNumber(2) val value: String = "",
    ) : IOneOfFieldName
    @Serializable data class LinuxTimestampField (
      @ProtoNumber(3) val value: Long = 0L,
    ) : IOneOfFieldName
  }
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as MessageName
    return true &&
      scalarFieldName.equals(other.scalarFieldName) &&
      oneOfFieldName.equals(other.oneOfFieldName)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + scalarFieldName.hashCode()
    result = 31 * result + oneOfFieldName.hashCode()
    return result
  }
}
