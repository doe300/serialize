/*
 * Basic deserialization functionality.
 *
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */
#pragma once

#include "common.hpp"

#include <any>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <complex>
#include <cstdint>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace serialize {

  /**
   * Base deserializer concept.
   *
   * Any type adhering to this concept can be used as deserializer.
   *
   * NOTE: The parameters of the read() member function are output-parameters and therefore must be lvalue references.
   */
  template <typename T>
  concept Deserializer = requires(T obj) {
    obj.read(std::declval<bool&>());
    obj.read(std::declval<int8_t&>());
    obj.read(std::declval<uint8_t&>());
    obj.read(std::declval<int16_t&>());
    obj.read(std::declval<uint16_t&>());
    obj.read(std::declval<int32_t&>());
    obj.read(std::declval<uint32_t&>());
    obj.read(std::declval<int64_t&>());
    obj.read(std::declval<uint64_t&>());
    obj.read(std::declval<float&>());
    obj.read(std::declval<double&>());
    obj.read(std::declval<long double&>());
    obj.read(std::declval<char&>());
    obj.read(std::declval<wchar_t&>());
    obj.read(std::declval<char16_t&>());
    obj.read(std::declval<char32_t&>());
    obj.read(std::declval<char8_t&>());
  };

  /**
   * Helper function to deserialize into an existing object.
   */
  template <typename T, Deserializer D> void deserializeInto(D& deserializer, T& out);

  namespace detail {
    /**
     * Helper type for basic deserialization, directly calling the corresponding Deserializer member function.
     */
    template <typename T> struct BasicDeserializerCall {
      template <Deserializer D> T operator()(D& deserializer) const {
        std::remove_const_t<T> tmp{};
        deserializer.read(tmp);
        return tmp;
      }
    };

    /**
     * Helper type to disable deserialization for specific types.
     */
    struct DisabledDeserializerCall {
      template <Deserializer D> void operator()(D& deserializer) const = delete;
    };

    [[noreturn]] inline void throwOnEof() { throw std::out_of_range{"Unexpected EOF while deserializing data"}; }
  } // namespace detail

  // Fundamental types
  /**
   * Main deserialization function.
   *
   * Returns an object of the template type parameter as read using the given deserializer object.
   *
   * The generic overload supports all types which can be directly read via Deserializer#read.
   */
  template <typename T> static constexpr detail::BasicDeserializerCall<T> deserialize;
  template <>
  inline constexpr auto deserialize<std::byte> =
      [](Deserializer auto& deserializer) { return std::bit_cast<std::byte>(deserialize<uint8_t>(deserializer)); };

  // Explicitly disable non-owning reference types
  template <typename T> static constexpr detail::DisabledDeserializerCall deserialize<T*>;
  template <typename T> static constexpr detail::DisabledDeserializerCall deserialize<T&>;
  template <typename T> static constexpr detail::DisabledDeserializerCall deserialize<std::span<T>>;
  template <typename T> static constexpr detail::DisabledDeserializerCall deserialize<std::shared_ptr<T>>;
  template <> inline constexpr detail::DisabledDeserializerCall deserialize<std::string_view>;
  // There is no guaranteed way to map the stored type to some serializable value and back
  template <> inline constexpr detail::DisabledDeserializerCall deserialize<std::any>;
  // Cannot return a copy of a deserialized C array
  template <typename T, std::size_t N> static constexpr detail::DisabledDeserializerCall deserialize<T[N]>;

  // Common standard library types

  template <typename T>
  static constexpr auto deserialize<std::atomic<T>> =
      [](Deserializer auto& deserializer) { return std::atomic<T>{deserialize<T>(deserializer)}; };

  template <typename R, typename P>
  static constexpr auto deserialize<std::chrono::duration<R, P>> =
      [](Deserializer auto& deserializer) { return std::chrono::duration<R, P>{deserialize<R>(deserializer)}; };

  template <typename C, typename Dur>
  static constexpr auto deserialize<std::chrono::time_point<C, Dur>> =
      [](Deserializer auto& deserializer) { return std::chrono::time_point<C, Dur>{deserialize<Dur>(deserializer)}; };

  template <typename T>
  static constexpr auto deserialize<std::complex<T>> = [](Deserializer auto& deserializer) {
    auto real = deserialize<T>(deserializer);
    auto imag = deserialize<T>(deserializer);
    return std::complex<T>{real, imag};
  };

  template <typename T>
  static constexpr auto deserialize<std::optional<T>> = [](Deserializer auto& deserializer) {
    if (deserialize<bool>(deserializer)) {
      return std::optional<T>{deserialize<T>(deserializer)};
    }
    return std::optional<T>{};
  };

  template <typename T, std::size_t N>
  static constexpr auto deserialize<std::array<T, N>> = [](Deserializer auto& deserializer) {
    std::array<T, N> result{};
    auto resultSize = deserialize<std::size_t>(deserializer);
    for (std::size_t i = 0; i < resultSize; ++i) {
      result.at(i) = deserialize<T>(deserializer);
    }
    return result;
  };

  /**
   * Concept for a deserializable growable container (e.g. a std::vector or std::map of deserializable element types).
   */
  template <typename T>
  concept DeserializableGrowableContainer =
      std::ranges::sized_range<T> &&
      (
          // e.g. std::set, std::map
          requires(T obj) { obj.emplace(std::declval<std::ranges::range_value_t<T>>()); } ||
          // e.g. std::vector, std::list
          requires(T obj) { obj.push_back(std::declval<std::ranges::range_value_t<T>>()); });

  /**
   * Deserialize any growable container (e.g. std::map, std::set std::string, std::unordered_set, std::vector,
   * std::list)
   */
  template <DeserializableGrowableContainer C>
  static constexpr auto deserialize<C> = [](Deserializer auto& deserializer) {
    using ValueType = std::ranges::range_value_t<C>;
    using SizeType = decltype(std::ranges::size(std::declval<C>()));
    C result{};
    auto resultSize = deserialize<SizeType>(deserializer);
    if constexpr (requires(C obj) { obj.reserve(std::declval<SizeType>()); }) {
      result.reserve(resultSize);
    }
    for (SizeType i = 0; i < resultSize; ++i) {
      if constexpr (requires(C obj) { obj.emplace(std::declval<ValueType>()); }) {
        result.emplace(deserialize<ValueType>(deserializer));
      } else {
        result.push_back(deserialize<ValueType>(deserializer));
      }
    }
    return result;
  };

  /**
   * Deserialize a std::tuple.
   *
   * NOTE: The current implementation requires a default-constructible tuple type!
   */
  template <typename... Args>
  static constexpr auto deserialize<std::tuple<Args...>> = [](Deserializer auto& deserializer) {
    std::tuple<Args...> result{};
    // TODO this requires a default-constructible tuple, but can't use
    // std::make_tuple(deserialize<Args>(deserializer)...), since the order is not guaranteed!
    std::apply([&deserializer](auto&... args) { (deserializeInto(deserializer, args), ...); }, result);
    return result;
  };

  template <typename F, typename S>
  static constexpr auto deserialize<std::pair<F, S>> = [](Deserializer auto& deserializer) {
    auto first = deserialize<F>(deserializer);
    auto second = deserialize<S>(deserializer);
    return std::make_pair(std::move(first), std::move(second));
  };

  template <typename T>
  static constexpr auto deserialize<std::unique_ptr<T>> = [](Deserializer auto& deserializer) {
    if (deserialize<bool>(deserializer)) {
      return std::make_unique<T>(deserialize<T>(deserializer));
    }
    return std::unique_ptr<T>{};
  };

  /**
   * Deserialize a std::variant.
   *
   * NOTE: The current implementation requires a default-constructible variant type!
   */
  template <typename... Args>
  static constexpr auto deserialize<std::variant<Args...>> = [](Deserializer auto& deserializer) {
    auto index = deserialize<std::size_t>(deserializer);
    if (index == std::variant_npos) {
      // Can't construct such a variant
      throw std::runtime_error{"Cannot deserialize valueless_by_exception variant object"};
    }
    // TODO requires default-constructible variant
    std::variant<Args...> result{};
    [&deserializer, &result, index]<std::size_t... Indices>(std::index_sequence<Indices...>) {
      auto deserializeIndex = [&deserializer, &result, index]<typename T, std::size_t Index>() {
        if (index == Index) {
          result.template emplace<Index>(deserialize<T>(deserializer));
        }
      };
      (deserializeIndex.template operator()<Args, Indices>(), ...);
    }(std::index_sequence_for<Args...>{});
    return result;
  };

  template <std::size_t N>
  static constexpr auto deserialize<std::bitset<N>> = [](Deserializer auto& deserializer) {
    if constexpr (!std::is_same_v<detail::EnclosingUnsignedType<N>, void>) {
      // can read as integral
      return std::bitset<N>{deserialize<detail::EnclosingUnsignedType<N>>(deserializer)};
    } else {
      // need to read as chunks of bits
      std::bitset<N> result{};
      uint8_t tmp = 0;
      for (std::size_t i = 0; i < N; ++i) {
        if (i % 8 == 0) {
          tmp = deserialize<uint8_t>(deserializer);
        }
        result.set(i, tmp & (1 << i % CHAR_BIT));
      }
      return result;
    }
  };

  namespace detail {

    // LCOV_EXCL_START
    /**
     * Dummy type providing a minimum implementation of the Deserializer concept, for type-checking only!
     */
    struct DummyDeserializer {
      void read(bool&) {}
      void read(int8_t&) {}
      void read(uint8_t&) {}
      void read(int16_t&) {}
      void read(uint16_t&) {}
      void read(int32_t&) {}
      void read(uint32_t&) {}
      void read(int64_t&) {}
      void read(uint64_t&) {}
      void read(float&) {}
      void read(double&) {}
      void read(long double&) {}
      void read(char&) {}
      void read(wchar_t&) {}
      void read(char16_t&) {}
      void read(char32_t&) {}
      void read(char8_t&) {}
    };
    // LCOV_EXCL_STOP

    static_assert(Deserializer<DummyDeserializer>);

    /**
     * Concept for class types providing a member deserialize() function.
     */
    template <typename T>
    concept MemberDeserializable = requires(T obj, DummyDeserializer deserializer) { obj.deserialize(deserializer); };

    /**
     * Concept for class types providing a static deserialize() function.
     */
    template <typename T>
    concept StaticMemberDeserializable =
        requires(T obj, DummyDeserializer deserializer) { T::deserialize(deserializer, obj); };

    /**
     * Concept for aggregate types which are not handled by any of the other cases and can be assigned via structured
     * bindings.
     */
    template <typename T>
    concept StructuredBindingDeserializable =
        !std::is_fundamental_v<T> && !DeserializableGrowableContainer<T> && !MemberDeserializable<T> &&
        !StaticMemberDeserializable<T> && std::is_standard_layout_v<T> && requires(T obj) {
          detail::forEachMember(obj, [](auto& mem) { deserializeInto(std::declval<DummyDeserializer&>(), mem); });
        };
  } // namespace detail

  /**
   * Deserialize any type with member deserialize() function.
   */
  template <detail::MemberDeserializable T>
  static constexpr auto deserialize<T> = [](Deserializer auto& deserializer) {
    std::remove_reference_t<T> tmp{};
    tmp.deserialize(deserializer);
    return tmp;
  };

  /**
   * Deserialize any type with static member deserialize() function.
   */
  template <detail::StaticMemberDeserializable T>
  static constexpr auto deserialize<T> = [](Deserializer auto& deserializer) {
    std::remove_reference_t<T> tmp{};
    T::deserialize(deserializer, tmp);
    return tmp;
  };

  /**
   * Deserialize "any" other standard layout type via structured binding to the members.
   *
   * NOTE: The current implementation requires a default-constructible type!
   */
  template <detail::StructuredBindingDeserializable T>
  static constexpr auto deserialize<T> = [](Deserializer auto& deserializer) {
    std::remove_reference_t<T> tmp{};
    detail::forEachMember(tmp, [&deserializer](auto& member) { deserializeInto(deserializer, member); });
    return tmp;
  };

  template <typename T, Deserializer D> void deserializeInto(D& deserializer, T& out) {
    out = deserialize<T>(deserializer);
  }
} // namespace serialize
