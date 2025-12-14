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
    return true &&
      floatValue.equals(other.floatValue) &&
      doubleValue.equals(other.doubleValue)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + floatValue.hashCode()
    result = 31 * result + doubleValue.hashCode()
    return result
  }
}
@Serializable data class Integer(
  @ProtoNumber(1) val variableSignedInt32: Int = 0,
  @ProtoNumber(2) val variableSignedInt64: Long = 0L,
  @ProtoNumber(3) val variableUnsignedInt32: UInt = 0U,
  @ProtoNumber(4) val variableUnsignedInt64: ULong = 0UL,
  @ProtoNumber(5) @ProtoType(ProtoIntegerType.FIXED) val fixedUnsignedInt32: UInt = 0U,
  @ProtoNumber(6) @ProtoType(ProtoIntegerType.FIXED) val fixedUnsignedInt64: ULong = 0UL,
  @ProtoNumber(7) @ProtoType(ProtoIntegerType.SIGNED) val zigzagSignedInt32: Int = 0,
  @ProtoNumber(8) @ProtoType(ProtoIntegerType.SIGNED) val zigzagSignedInt64: Long = 0L,
) {
  override fun equals(other: Any?): Boolean {
    if (this === other) return true
    if (javaClass != other?.javaClass) return false
    other as Integer
    return true &&
      variableSignedInt32.equals(other.variableSignedInt32) &&
      variableSignedInt64.equals(other.variableSignedInt64) &&
      variableUnsignedInt32.equals(other.variableUnsignedInt32) &&
      variableUnsignedInt64.equals(other.variableUnsignedInt64) &&
      fixedUnsignedInt32.equals(other.fixedUnsignedInt32) &&
      fixedUnsignedInt64.equals(other.fixedUnsignedInt64) &&
      zigzagSignedInt32.equals(other.zigzagSignedInt32) &&
      zigzagSignedInt64.equals(other.zigzagSignedInt64)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + variableSignedInt32.hashCode()
    result = 31 * result + variableSignedInt64.hashCode()
    result = 31 * result + variableUnsignedInt32.hashCode()
    result = 31 * result + variableUnsignedInt64.hashCode()
    result = 31 * result + fixedUnsignedInt32.hashCode()
    result = 31 * result + fixedUnsignedInt64.hashCode()
    result = 31 * result + zigzagSignedInt32.hashCode()
    result = 31 * result + zigzagSignedInt64.hashCode()
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
    return true &&
      boolean.equals(other.boolean)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + boolean.hashCode()
    return result
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
    return true &&
      bytesSequence.contentEquals(other.bytesSequence) &&
      charsSequence.equals(other.charsSequence)
  }
  override fun hashCode(): Int {
    var result = 0
    result = 31 * result + bytesSequence.contentHashCode()
    result = 31 * result + charsSequence.hashCode()
    return result
  }
}
