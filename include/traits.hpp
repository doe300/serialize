/*
 * Trait types and concepts.
 *
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */
#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"

namespace serialize {

  namespace detail {
    // LCOV_EXCL_START
    /**
     * Dummy type providing a minimum implementation of the Serializer concept, for type-checking only!
     */
    struct DummySerializer {
      void write(bool);
      void write(int8_t);
      void write(uint8_t);
      void write(int16_t);
      void write(uint16_t);
      void write(int32_t);
      void write(uint32_t);
      void write(int64_t);
      void write(uint64_t);
      void write(float);
      void write(double);
      void write(long double);
      void write(char);
      void write(wchar_t);
      void write(char16_t);
      void write(char32_t);
      void write(char8_t);
      void write(std::size_t, std::span<const std::byte>);

      void flush();
    };
    // LCOV_EXCL_STOP

    static_assert(Serializer<DummySerializer>);

  } // namespace detail

  /**
   * Concept for any generally serializable type, i.e. any type which can be serialized by any Serializer.
   */
  template <typename T>
  concept Serializable = requires(const T obj) { serialize::serialize(std::declval<detail::DummySerializer&>(), obj); };

  /**
   * Concept for any generally deserializable type, i.e. any type which can be deserialized by any Deerializer.
   */
  template <typename T>
  concept Deserializable = requires() {
    { serialize::deserialize<T>(std::declval<detail::DummyDeserializer&>()) } -> std::same_as<T>;
  };
} // namespace serialize
