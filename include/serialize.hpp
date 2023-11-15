#pragma once

#include "common.hpp"

#include <atomic>
#include <bitset>
#include <chrono>
#include <complex>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace serialize {

  /**
   * Base serializer concept.
   *
   * Any type adhering to this concept can be used as serializer.
   *
   */
  template <typename T>
  concept Serializer = requires(T obj) {
    obj.write(std::declval<bool>());
    obj.write(std::declval<int8_t>());
    obj.write(std::declval<uint8_t>());
    obj.write(std::declval<int16_t>());
    obj.write(std::declval<uint16_t>());
    obj.write(std::declval<int32_t>());
    obj.write(std::declval<uint32_t>());
    obj.write(std::declval<int64_t>());
    obj.write(std::declval<uint64_t>());
    obj.write(std::declval<float>());
    obj.write(std::declval<double>());
    obj.write(std::declval<long double>());
    obj.write(std::declval<char>());
    obj.write(std::declval<wchar_t>());
    obj.write(std::declval<char16_t>());
    obj.write(std::declval<char32_t>());
    obj.write(std::declval<char8_t>());

    /**
     * Flush the underlying output after all data has been serialized. May be no-op, if flushing is not required.
     */
    obj.flush();
  };

  /**
   * Extension of the Serializer allowing for more efficient serialization of continuous memory ranges.
   */
  template <typename T>
  concept ByteSerializer = Serializer<T> && requires(T obj) {
    /**
     * Prototype for a function taking the number of bytes and the byte range as arguments.
     */
    obj.write(std::declval<std::size_t>(), std::declval<std::span<const std::byte>>());
  };

  // Fundamental types
  template <Serializer S> void serialize(S& serializer, bool b) { serializer.write(b); }
  template <Serializer S> void serialize(S& serializer, int8_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, uint8_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, int16_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, uint16_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, int32_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, uint32_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, int64_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, uint64_t i) { serializer.write(i); }
  template <Serializer S> void serialize(S& serializer, float f) { serializer.write(f); }
  template <Serializer S> void serialize(S& serializer, double f) { serializer.write(f); }
  template <Serializer S> void serialize(S& serializer, long double f) { serializer.write(f); }
  template <Serializer S> void serialize(S& serializer, char c) { serializer.write(c); }
  template <Serializer S> void serialize(S& serializer, wchar_t c) { serializer.write(c); }
  template <Serializer S> void serialize(S& serializer, char16_t c) { serializer.write(c); }
  template <Serializer S> void serialize(S& serializer, char32_t c) { serializer.write(c); }
  template <Serializer S> void serialize(S& serializer, char8_t c) { serializer.write(c); }
  template <Serializer S> void serialize(S& serializer, std::byte b) { serializer.write(std::bit_cast<uint8_t>(b)); }

  // Common standard library types

  template <Serializer S, typename T> void serialize(S& serializer, const std::atomic<T>& atomic) {
    serialize(serializer, atomic.load());
  }

  template <Serializer S, typename R, typename P>
  void serialize(S& serializer, const std::chrono::duration<R, P>& duration) {
    serialize(serializer, duration.count());
  }

  template <Serializer S, typename C, typename D>
  void serialize(S& serializer, const std::chrono::time_point<C, D>& time) {
    serialize(serializer, time.time_since_epoch().count());
  }

  template <Serializer S, typename T> void serialize(S& serializer, const std::complex<T>& complex) {
    serialize(serializer, complex.real());
    serialize(serializer, complex.imag());
  }

  template <Serializer S, typename T> void serialize(S& serializer, const std::optional<T>& option) {
    serialize(serializer, option.has_value());
    if (option) {
      serialize(serializer, option.value());
    }
  }

  /**
   * Concept for a contiguous range of trivial values allowing for "raw memory" serialization via a ByteSerializer.
   */
  template <typename T>
  concept SerializableRawData = std::ranges::contiguous_range<T> && std::ranges::sized_range<T> &&
                                requires(T obj) { requires std::is_trivial_v<std::ranges::range_value_t<T>>; };

  /**
   * Serialize for sized iterable containers containing trivial types (e.g. std::array, std::string, std::vector with
   * integral elements) via a ByteSerializer by using the more efficient raw memory serialization function.
   */
  template <ByteSerializer S, SerializableRawData C> void serialize(S& serializer, const C& container) {
    serializer.write(std::ranges::size(container),
                     std::as_bytes(std::span<const std::ranges::range_value_t<C>>{container}));
  }

  template <Serializer S, SerializableRawData C> void serialize(S& serializer, const C& container) {
    serialize(serializer, std::ranges::size(container));
    for (const auto& entry : container) {
      serialize(serializer, entry);
    }
  }

  /**
   * Concept for any other serializable container (not matching SerializableRawData).
   */
  template <typename T>
  concept SerializableContainer =
      !SerializableRawData<T> && std::ranges::common_range<T> && std::ranges::sized_range<T>;

  /**
   * Serialize any other sized iterable containers (e.g. std::array, std::map, std::set std::string, std::unordered_set,
   * std::vector).
   */
  template <Serializer S, SerializableContainer C> void serialize(S& serializer, const C& container) {
    serialize(serializer, std::ranges::size(container));
    for (const auto& entry : container) {
      serialize(serializer, entry);
    }
  }

  /**
   * Serialize any tuple-like types (e.g. std::tuple, std::pair).
   */
  template <Serializer S, detail::TupleType T> void serialize(S& serializer, const T& tuple) {
    std::apply([&serializer](const auto&... args) { (serialize(serializer, args), ...); }, tuple);
  }

  template <Serializer S, typename T> void serialize(S& serializer, const std::unique_ptr<T>& ptr) {
    serialize(serializer, static_cast<bool>(ptr));
    if (ptr) {
      serialize(serializer, *ptr);
    }
  }

  template <Serializer S, typename... Args> void serialize(S& serializer, const std::variant<Args...>& variant) {
    serialize(serializer, variant.index());
    std::visit([&serializer](const auto& obj) { serialize(serializer, obj); }, variant);
  }

  template <Serializer S, std::size_t N> void serialize(S& serializer, const std::bitset<N>& bits) {
    if constexpr (!std::is_same_v<detail::EnclosingUnsignedType<N>, void>) {
      // can store as integral
      serialize(serializer, static_cast<detail::EnclosingUnsignedType<N>>(bits.to_ullong()));
    } else {
      // need to store as chunks of bits
      uint8_t tmp = 0;
      for (std::size_t i = 0; i < N; ++i) {
        if (i > 0 && i % 8 == 0) {
          serialize(serializer, tmp);
          tmp = 0;
        }
        tmp |= (bits.test(i) ? 1 : 0) << (i % 8);
      }
      if (N % 8) {
        serialize(serializer, tmp);
      }
    }
  }

  namespace detail {
    template <Serializer S, typename T>
    constexpr bool is_member_serializable = requires(const T obj, S serializer) { obj.serialize(serializer); };

    template <Serializer S, typename T>
    constexpr bool is_static_member_serializable =
        requires(const T obj, S serializer) { T::serialize(serializer, obj); };

    template <Serializer S, typename T>
    constexpr bool is_structured_bindings_serializable =
        !std::is_fundamental_v<T> && std::is_standard_layout_v<T> && !is_member_serializable<S, T> &&
        !is_static_member_serializable<S, T> &&
        requires(T obj) { detail::forEachMember(obj, [](auto member) { serialize(std::declval<S&>(), member); }); };
  } // namespace detail

  /**
   * Serialize any type with member serialize() function.
   */
  template <Serializer S, typename T>
  std::enable_if_t<detail::is_member_serializable<S, T>> serialize(S& serializer, const T& object) {
    object.serialize(serializer);
  }

  /**
   * Serialize any type with static member serialize() function.
   */
  template <Serializer S, typename T>
  std::enable_if_t<detail::is_static_member_serializable<S, T>> serialize(S& serializer, const T& object) {
    T::serialize(serializer, object);
  }

  /**
   * Serialize "any" other standard layout type via structured binding to the members.
   */
  template <Serializer S, typename T>
  std::enable_if_t<detail::is_structured_bindings_serializable<S, T>> serialize(S& serializer, const T& object) {
    detail::forEachMember(object, [&serializer](auto member) { serialize(serializer, member); });
  }
} // namespace serialize
