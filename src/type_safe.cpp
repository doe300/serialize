/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "type_safe.hpp"
#include "simple.hpp"

namespace serialize {

  static_assert(Serializer<TypeSafeSerializer<SimpleStreamSerializer>>);
  static_assert(!ByteSerializer<TypeSafeSerializer<SimpleStreamSerializer>>);
  static_assert(Deserializer<TypeSafeDeserializer<SimpleStreamDeserializer>>);

  namespace detail {

    static std::string getTypeName(uint8_t typeId) {
#define TYPE(Name)                                                                                                     \
  case type_id_v<Name>:                                                                                                \
    return #Name
      switch (typeId) {
        // This switch-case also guarantees the type IDs to be unique
        TYPE(bool);
        TYPE(int8_t);
        TYPE(int16_t);
        TYPE(uint16_t);
        TYPE(int32_t);
        TYPE(uint32_t);
        TYPE(int64_t);
        TYPE(uint64_t);
        TYPE(float);
        TYPE(double);
        TYPE(long double);
        TYPE(char);
        TYPE(wchar_t);
        TYPE(char16_t);
        TYPE(char32_t);
        TYPE(char8_t);
      default:
        return "unknown";
      }
#undef TYPE
    }

    void throwOnTypeMismatch(uint8_t expectedTypeId, uint8_t actualTypeId) {
      throw std::domain_error{"Invalid type in data stream, expected '" + getTypeName(expectedTypeId) + "', got '" +
                              getTypeName(actualTypeId) + "'"};
    }
  } // namespace detail

} // namespace serialize
