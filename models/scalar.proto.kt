package kts.scalar
import kotlinx.serialization.Serializable
import kotlinx.serialization.protobuf.*
@Serializable data class Decimal(
  @ProtoNumber(1) val floatValue: Float = 0F,
  @ProtoNumber(2) val doubleValue: Double = 0.0,
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Decimal
    return floatValue.equals(other.floatValue) &&
      doubleValue.equals(other.doubleValue)
  }
  override fun hashCode(): Int {
    var result = floatValue.hashCode()
    result = 31 * result + doubleValue.hashCode()
    return result
  }
}
@Serializable data class Integer(
  @ProtoNumber(1) @ProtoType(ProtoIntegerType.DEFAULT) val signedVariableInt32: Int = 0,
  @ProtoNumber(2) @ProtoType(ProtoIntegerType.DEFAULT) val signedVariableInt64: Long = 0L,
  @ProtoNumber(5) @ProtoType(ProtoIntegerType.SIGNED) val signedZigzagInt32: Int = 0,
  @ProtoNumber(6) @ProtoType(ProtoIntegerType.SIGNED) val signedZigzagInt64: Long = 0L,
  @ProtoNumber(7) @ProtoType(ProtoIntegerType.FIXED) val unsignedFixedInt32: UInt = 0U,
  @ProtoNumber(8) @ProtoType(ProtoIntegerType.FIXED) val unsignedFixedInt64: ULong = 0UL,
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Integer
    return signedVariableInt32.equals(other.signedVariableInt32) &&
      signedVariableInt64.equals(other.signedVariableInt64) &&
      signedZigzagInt32.equals(other.signedZigzagInt32) &&
      signedZigzagInt64.equals(other.signedZigzagInt64) &&
      unsignedFixedInt32.equals(other.unsignedFixedInt32) &&
      unsignedFixedInt64.equals(other.unsignedFixedInt64)
  }
  override fun hashCode(): Int {
    var result = signedVariableInt32.hashCode()
    result = 31 * result + signedVariableInt64.hashCode()
    result = 31 * result + signedZigzagInt32.hashCode()
    result = 31 * result + signedZigzagInt64.hashCode()
    result = 31 * result + unsignedFixedInt32.hashCode()
    result = 31 * result + unsignedFixedInt64.hashCode()
    return result
  }
}
@Serializable data class Logical(
  @ProtoNumber(1) val boolean: Boolean = false,
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Logical
    return boolean.equals(other.boolean)
  }
  override fun hashCode(): Int {
    return boolean.hashCode()
  }
}
@Serializable data class Sequence(
  @ProtoNumber(1) val bytesSequence: ByteArray = ByteArray(0),
  @ProtoNumber(2) val charsSequence: String = "",
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Sequence
    return bytesSequence.contentEquals(other.bytesSequence) &&
      charsSequence.equals(other.charsSequence)
  }
  override fun hashCode(): Int {
    var result = bytesSequence.contentHashCode()
    result = 31 * result + charsSequence.hashCode()
    return result
  }
}
