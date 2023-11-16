/*
 * Common helper and utility symbols.
 *
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */
#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace serialize {

  namespace detail {
    template <typename T> struct is_tuple_type : std::false_type {};
    template <typename... T> struct is_tuple_type<std::tuple<T...>> : std::true_type {};
    template <typename L, typename R> struct is_tuple_type<std::pair<L, R>> : std::true_type {};

    // TODO any type for which std::tuple_size / std::get is defined, see Notes in
    // https://en.cppreference.com/w/cpp/utility/apply

    template <typename T>
    concept TupleType = is_tuple_type<T>::value;

    template <typename T> struct is_fixed_size_container : std::false_type {};
    template <typename T, std::size_t N> struct is_fixed_size_container<std::array<T, N>> : std::true_type {};
    template <typename T, std::size_t N> struct is_fixed_size_container<T[N]> : std::true_type {};
    template <typename T> constexpr bool is_fixed_size_container_v = is_fixed_size_container<T>::value;

    template <std::size_t NumBits>
    using EnclosingUnsignedType = std::conditional_t<
        NumBits <= std::numeric_limits<uint8_t>::digits, uint8_t,
        std::conditional_t<
            NumBits <= std::numeric_limits<uint16_t>::digits, uint16_t,
            std::conditional_t<NumBits <= std::numeric_limits<uint32_t>::digits, uint32_t,
                               std::conditional_t<NumBits <= std::numeric_limits<uint64_t>::digits, uint64_t, void>>>>;

    struct AnyType {
      template <class T> constexpr operator T(); // non explicit
    };

    // Adapted from https://stackoverflow.com/a/40001277
    template <typename T, std::size_t N, typename = std::make_index_sequence<N>> struct HasMembers;

    template <typename T, std::size_t N, std::size_t... Sequence>
    struct HasMembers<T, N, std::index_sequence<Sequence...>> {
      template <std::size_t> using type = AnyType;

      static constexpr bool value = requires { std::decay_t<T>{std::declval<type<Sequence>>()...}; };
    };

    // accepts less members -> need to check for large to small
    static_assert(!HasMembers<std::pair<int, int>, 3>::value);
    static_assert(HasMembers<std::pair<int, int>, 2>::value);
    static_assert(HasMembers<std::pair<int, int>, 1>::value);

    template <typename Func, typename... Args> void applyAll(Func&& func, Args&&... args) { (..., func(args)); }

    // Adapted from https://www.reddit.com/r/cpp/comments/4yp7fv/c17_structured_bindings_convert_struct_to_a_tuple/
    template <typename T, typename Func> void forEachMember(T&& object, Func&& func) noexcept {
      if constexpr (HasMembers<T, 20>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17,
                 p18, p19, p20);
      } else if constexpr (HasMembers<T, 19>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17,
                 p18, p19);
      } else if constexpr (HasMembers<T, 18>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17,
                 p18);
      } else if constexpr (HasMembers<T, 17>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
      } else if constexpr (HasMembers<T, 16>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
      } else if constexpr (HasMembers<T, 15>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
      } else if constexpr (HasMembers<T, 14>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
      } else if constexpr (HasMembers<T, 13>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
      } else if constexpr (HasMembers<T, 12>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
      } else if constexpr (HasMembers<T, 11>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
      } else if constexpr (HasMembers<T, 10>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
      } else if constexpr (HasMembers<T, 9>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8, p9] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8, p9);
      } else if constexpr (HasMembers<T, 8>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7, p8] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7, p8);
      } else if constexpr (HasMembers<T, 7>::value) {
        auto&& [p1, p2, p3, p4, p5, p6, p7] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6, p7);
      } else if constexpr (HasMembers<T, 6>::value) {
        auto&& [p1, p2, p3, p4, p5, p6] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5, p6);
      } else if constexpr (HasMembers<T, 5>::value) {
        auto&& [p1, p2, p3, p4, p5] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4, p5);
      } else if constexpr (HasMembers<T, 4>::value) {
        auto&& [p1, p2, p3, p4] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3, p4);
      } else if constexpr (HasMembers<T, 3>::value) {
        auto&& [p1, p2, p3] = object;
        applyAll(std::forward<Func>(func), p1, p2, p3);
      } else if constexpr (HasMembers<T, 2>::value) {
        auto&& [p1, p2] = object;
        applyAll(std::forward<Func>(func), p1, p2);
      } else if constexpr (HasMembers<T, 1>::value) {
        auto&& [p1] = object;
        applyAll(std::forward<Func>(func), p1);
      }
    }
  } // namespace detail
} // namespace serialize
