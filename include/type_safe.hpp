#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace serialize {

  namespace detail {
    template <typename T> struct type_id {};

    template <> struct type_id<bool> : std::integral_constant<uint8_t, 0> {};
    template <> struct type_id<int8_t> : std::integral_constant<uint8_t, 1> {};
    template <> struct type_id<uint8_t> : std::integral_constant<uint8_t, 2> {};
    template <> struct type_id<int16_t> : std::integral_constant<uint8_t, 3> {};
    template <> struct type_id<uint16_t> : std::integral_constant<uint8_t, 4> {};
    template <> struct type_id<int32_t> : std::integral_constant<uint8_t, 5> {};
    template <> struct type_id<uint32_t> : std::integral_constant<uint8_t, 6> {};
    template <> struct type_id<int64_t> : std::integral_constant<uint8_t, 7> {};
    template <> struct type_id<uint64_t> : std::integral_constant<uint8_t, 8> {};
    template <> struct type_id<float> : std::integral_constant<uint8_t, 9> {};
    template <> struct type_id<double> : std::integral_constant<uint8_t, 10> {};
    template <> struct type_id<long double> : std::integral_constant<uint8_t, 11> {};
    template <> struct type_id<char> : std::integral_constant<uint8_t, 12> {};
    template <> struct type_id<wchar_t> : std::integral_constant<uint8_t, 13> {};
    template <> struct type_id<char8_t> : std::integral_constant<uint8_t, 14> {};
    template <> struct type_id<char16_t> : std::integral_constant<uint8_t, 15> {};
    template <> struct type_id<char32_t> : std::integral_constant<uint8_t, 16> {};

    template <typename T> static constexpr uint8_t type_id_v = type_id<std::decay_t<T>>::value;

    [[noreturn]] void throwOnTypeMismatch(uint8_t expectedTypeId, uint8_t actualTypeId);
  } // namespace detail

  /**
   * Type-safe wrapper around any other Serializer.
   *
   * The type safety is achieved by serializing a type-id for every value serialized.
   */
  template <Serializer Inner> class TypeSafeSerializer {
  public:
    explicit TypeSafeSerializer(Inner& inner) : inner(inner) {}

    explicit TypeSafeSerializer(Inner&& inner)
        : innerHolder(std::make_unique<Inner>(std::move(inner))), inner(*innerHolder) {}

    explicit TypeSafeSerializer(std::unique_ptr<Inner>&& inner) : innerHolder(std::move(inner)), inner(*innerHolder) {
      if (!innerHolder)
        throw std::invalid_argument{"Cannot wrap a NULL serializer object"};
    }

    template <typename T> std::enable_if_t<std::is_fundamental_v<T>> write(T val) {
      inner.write(detail::type_id_v<T>);
      inner.write(val);
    }

    void flush() { inner.flush(); }

  private:
    std::unique_ptr<Inner> innerHolder;
    Inner& inner;
  };

  /**
   * Type-safe wrapper around any other Deserializer.
   *
   * The type-id written by the TypeSafeSerializer is checked on deserialization and an exception thrown on mismatch.
   */
  template <Deserializer Inner> class TypeSafeDeserializer {
  public:
    explicit TypeSafeDeserializer(Inner& inner) : inner(inner) {}

    explicit TypeSafeDeserializer(Inner&& inner)
        : innerHolder(std::make_unique<Inner>(std::move(inner))), inner(*innerHolder) {}

    explicit TypeSafeDeserializer(std::unique_ptr<Inner>&& inner) : innerHolder(std::move(inner)), inner(*innerHolder) {
      if (!innerHolder)
        throw std::invalid_argument{"Cannot wrap a NULL deserializer object"};
    }

    template <typename T> std::enable_if_t<std::is_fundamental_v<T>> read(T& val) {
      uint8_t typeId = 255;
      inner.read(typeId);
      if (typeId != detail::type_id_v<T>) {
        detail::throwOnTypeMismatch(detail::type_id_v<T>, typeId);
      }
      inner.read(val);
    }

  private:
    std::unique_ptr<Inner> innerHolder;
    Inner& inner;
  };

} // namespace serialize
