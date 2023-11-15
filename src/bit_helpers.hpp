#pragma once

#include "bit_packing.hpp"

#include <bit>
#include <climits>
#include <cstdint>
#include <limits>

namespace serialize {
  struct BitValue {
    uintmax_t value = 0;
    uint8_t numBits = 0;
  };

  constexpr BitValue encodeExpGolomb(uintmax_t value) noexcept {
    ++value;
    auto numBits = std::bit_width(value) - 1;
    return {value, static_cast<uint8_t>(numBits * 2 + 1)};
  }

  constexpr uintmax_t decodeExpGolomb(uintmax_t value) noexcept { return value - 1U; }

  constexpr BitValue encodeSignedExpGolomb(intmax_t value) noexcept {
    auto tmp = value < 0 ? (-2 * value) : value > 0 ? (2 * value - 1) : 0;
    return encodeExpGolomb(std::bit_cast<uintmax_t>(tmp));
  }

  constexpr intmax_t decodeSignedExpGolomb(uintmax_t value) noexcept {
    auto tmp = decodeExpGolomb(value);
    auto sign = (tmp + 1U) & 0x1 ? -1 : 1;
    auto val = static_cast<std::intmax_t>(tmp / 2 + (tmp & 0x1));
    return sign * val;
  }

  // Adapted from https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
  template <typename T> constexpr T reverseBits(T value) noexcept {
    T result = value;
    auto shifts = std::numeric_limits<T>::digits - 1;

    for (value >>= 1; value; value >>= 1) {
      result <<= 1;
      result |= value & 0x01;
      --shifts;
    }

    return result << shifts;
  }

  static_assert(sizeof(uint8_t) == sizeof(std::byte));

  static constexpr auto CACHE_SIZE = std::numeric_limits<uintmax_t>::digits;
  static_assert(sizeof(BitCache{}.value) * CHAR_BIT == CACHE_SIZE);

  template <typename Func = void (*)(std::byte)>
  static constexpr void flushFullBytes(BitCache& cache, Func&& sinkByte) {
    while (cache.usedBits >= CHAR_BIT) {
      // flush highest cache byte
      // cache is left-adjusted, i.e. high bits are first in, first out
      auto byte = static_cast<uint8_t>(cache.value >> (CACHE_SIZE - CHAR_BIT));
      cache.value <<= CHAR_BIT;
      cache.usedBits -= CHAR_BIT;
      sinkByte(std::bit_cast<std::byte>(byte));
    }
  }

  template <typename Func = void (*)(std::byte)>
  static constexpr void writeBits(BitCache& cache, Func&& sinkByte, BitValue value) {
    if (value.numBits + cache.usedBits > CACHE_SIZE) {
      // split up the bits to write into multiple calls
      auto lowerBits = value.numBits / 2U;
      auto upper = value.value >> lowerBits;
      writeBits(cache, sinkByte, {upper, static_cast<uint8_t>(value.numBits - lowerBits)});
      auto lower = value.value & ((std::uintmax_t{1} << lowerBits) - 1u);
      writeBits(cache, sinkByte, {lower, static_cast<uint8_t>(lowerBits)});
      return;
    } else if (value.numBits) {
      // cache is left-adjusted, i.e. high bits are first in, first out
      cache.usedBits += value.numBits;
      cache.value |= value.value << (CACHE_SIZE - cache.usedBits);
      flushFullBytes(cache, sinkByte);
    }
  }

  template <typename Func = bool (*)(std::byte&)>
  [[nodiscard]] static constexpr bool feedFullByte(BitCache& cache, Func&& sourceByte) {
    std::byte in{};
    if ((CACHE_SIZE - cache.usedBits) < CHAR_BIT || !sourceByte(in)) {
      return false;
    }
    auto byte = static_cast<std::uintmax_t>(in);
    byte <<= CACHE_SIZE - cache.usedBits - CHAR_BIT;
    cache.value |= byte;
    cache.usedBits += CHAR_BIT;
    return true;
  }

  template <typename Func = bool (*)(std::byte&)>
  static constexpr BitValue readExGolombBits(BitCache& cache, Func&& sourceByte) {
    uint32_t numLeadingZeroes = 0;
    while (!cache.value) {
      // To allow for more than CACHE_SIZE leading zeroes, collect leading zero bytes early
      numLeadingZeroes += cache.usedBits;
      cache.usedBits = 0;
      if (!feedFullByte(cache, sourceByte)) {
        return {};
      }
    }

    // extract number of leading zero-bits
    auto exponent = std::countl_zero(cache.value);
    cache.usedBits -= exponent;
    cache.value <<= exponent;
    auto numBits = static_cast<uint8_t>(numLeadingZeroes + exponent + 1 /* marker 1-bit */);

    // TODO due to limitation of output type, cannot read more the bits than fit into UINTMAX_T

    // fill actual data bits
    BitValue result{};
    while ((result.numBits + cache.usedBits) < numBits) {
      if (!feedFullByte(cache, sourceByte)) {
        return {};
      }
      if (cache.usedBits >= CACHE_SIZE / 2 && (numBits - result.numBits) > cache.usedBits) {
        // partially copy cache to result to not overflow when getting close to CACHE_SIZE bits
        result.value |= cache.value >> (CACHE_SIZE - cache.usedBits);
        result.numBits += cache.usedBits;
        cache.usedBits = 0;
        cache.value = 0;
      }
    }

    // extract value and update cache
    auto numRemainingBits = numBits - result.numBits;
    if (result.numBits) {
      result.value <<= numRemainingBits;
    }
    result.value |= cache.value >> (CACHE_SIZE - numRemainingBits);
    result.numBits += numRemainingBits;
    cache.usedBits -= numRemainingBits;
    cache.value <<= numRemainingBits;

    return result;
  }

  static_assert(encodeExpGolomb(0U).value == 0b1);
  static_assert(encodeExpGolomb(0U).numBits == 1);
  static_assert(encodeExpGolomb(1U).value == 0b010);
  static_assert(encodeExpGolomb(1U).numBits == 3);
  static_assert(encodeExpGolomb(8U).value == 0b0001001);
  static_assert(encodeExpGolomb(8U).numBits == 2 * 3 + 1);
  static_assert(encodeExpGolomb(17U).value == 0b000010010);
  static_assert(encodeExpGolomb(17U).numBits == 2 * 4 + 1);
  static_assert(encodeExpGolomb(42U).value == 0b00000101011);
  static_assert(encodeExpGolomb(42U).numBits == 2 * 5 + 1);

  static_assert(decodeExpGolomb(0b1U) == 0);
  static_assert(decodeExpGolomb(0b010U) == 1);
  static_assert(decodeExpGolomb(0b0001001U) == 8);
  static_assert(decodeExpGolomb(0b000010010U) == 17);
  static_assert(decodeExpGolomb(0b00000101011U) == 42);

  static_assert(encodeSignedExpGolomb(0).value == 0b1);
  static_assert(encodeSignedExpGolomb(0).numBits == 1);
  static_assert(encodeSignedExpGolomb(1).value == 0b010);
  static_assert(encodeSignedExpGolomb(1).numBits == 3);
  static_assert(encodeSignedExpGolomb(8).value == 0b000010000);
  static_assert(encodeSignedExpGolomb(8).numBits == 2 * 4 + 1);
  static_assert(encodeSignedExpGolomb(17).value == 0b00000100010);
  static_assert(encodeSignedExpGolomb(17).numBits == 2 * 5 + 1);
  static_assert(encodeSignedExpGolomb(42).value == 0b0000001010100);
  static_assert(encodeSignedExpGolomb(42).numBits == 2 * 6 + 1);
  static_assert(encodeSignedExpGolomb(-1).value == 0b011);
  static_assert(encodeSignedExpGolomb(-1).numBits == 3);
  static_assert(encodeSignedExpGolomb(-8).value == 0b000010001);
  static_assert(encodeSignedExpGolomb(-8).numBits == 2 * 4 + 1);
  static_assert(encodeSignedExpGolomb(-17).value == 0b00000100011);
  static_assert(encodeSignedExpGolomb(-17).numBits == 2 * 5 + 1);
  static_assert(encodeSignedExpGolomb(-42).value == 0b0000001010101);
  static_assert(encodeSignedExpGolomb(-42).numBits == 2 * 6 + 1);

  static_assert(decodeSignedExpGolomb(0b1U) == 0);
  static_assert(decodeSignedExpGolomb(0b010U) == 1);
  static_assert(decodeSignedExpGolomb(0b000010000U) == 8);
  static_assert(decodeSignedExpGolomb(0b00000100010U) == 17);
  static_assert(decodeSignedExpGolomb(0b0000001010100U) == 42);
  static_assert(decodeSignedExpGolomb(0b011U) == -1);
  static_assert(decodeSignedExpGolomb(0b000010001U) == -8);
  static_assert(decodeSignedExpGolomb(0b00000100011U) == -17);
  static_assert(decodeSignedExpGolomb(0b0000001010101U) == -42);

  static_assert(reverseBits(0) == 0);
  static_assert(reverseBits<uint16_t>(0b0000110011110000) == 0b0000111100110000);
  static_assert(reverseBits<uint8_t>(0b01001100) == 0b00110010);
  static_assert(reverseBits<uint64_t>(0x43CE4AA5435F3093) == 0xC90CFAC2A55273C2);
  static_assert(reverseBits<uint16_t>(reverseBits<uint16_t>(0b0000110011110000)) == 0b0000110011110000);
  static_assert(reverseBits<uint8_t>(reverseBits<uint8_t>(0b01001100)) == 0b01001100);
  static_assert(reverseBits<uint64_t>(reverseBits<uint64_t>(0x43CE4AA5435F3093)) == 0x43CE4AA5435F3093);

  namespace detail {
    struct CacheResult {
      uintmax_t value;
      uint8_t numBits;
      uintmax_t cacheValue;
      uint8_t cacheBits;

      constexpr bool operator<=>(const CacheResult& other) const noexcept = default;
    };

    static constexpr CacheResult testFlushFullBytes(BitCache cache) {
      uintmax_t result = 0;
      uint8_t numBits = 0;
      flushFullBytes(cache, [&](std::byte byte) {
        result <<= CHAR_BIT;
        result |= std::bit_cast<uint8_t>(byte);
        numBits += CHAR_BIT;
      });
      return CacheResult{result, numBits, cache.value, cache.usedBits};
    }

    static constexpr CacheResult testWriteBits(BitCache cache, BitValue value) {
      uintmax_t result = 0;
      uint8_t numBits = 0;
      writeBits(
          cache,
          [&](std::byte byte) {
            result <<= CHAR_BIT;
            result |= std::bit_cast<uint8_t>(byte);
            numBits += CHAR_BIT;
          },
          value);
      return CacheResult{result, numBits, cache.value, cache.usedBits};
    }

    static constexpr std::pair<std::array<uint8_t, 16>, BitCache> testWriteManyBits(BitValue value) {
      BitCache cache;
      std::array<uint8_t, 16> result;
      result.fill(0);
      uint8_t index = 0;
      auto sink = [&](std::byte byte) mutable { result[index++] = std::bit_cast<uint8_t>(byte); };
      writeBits(cache, sink, value);
      if (cache.usedBits) {
        sink(std::bit_cast<std::byte>(static_cast<uint8_t>(cache.value >> (CACHE_SIZE - CHAR_BIT))));
      }
      return std::make_pair(result, cache);
    }

    static constexpr BitCache testFeedFullByte(BitCache cache, uint8_t byteValue) {
      bool producedByte = false;
      auto status = feedFullByte(cache, [&](std::byte& out) {
        if (producedByte) {
          return false;
        }
        out = std::bit_cast<std::byte>(byteValue);
        producedByte = true;
        return true;
      });
      if (!status) {
        throw std::logic_error{""};
      }
      return cache;
    }

    static constexpr CacheResult testReadExpGolombBits(BitCache cache, BitValue value) {
      auto val = readExGolombBits(cache, [&value](std::byte& out) {
        if (!value.numBits) {
          return false;
        }
        auto byte =
            static_cast<uint8_t>(value.numBits <= CHAR_BIT ? value.value : value.value >> (value.numBits - CHAR_BIT));
        value.numBits = value.numBits < CHAR_BIT ? 0 : (value.numBits - CHAR_BIT);
        value.value &= (uintmax_t{1} << value.numBits) - 1U;
        out = std::bit_cast<std::byte>(byte);
        return true;
      });
      return {val.value, val.numBits, cache.value, cache.usedBits};
    }

    static constexpr CacheResult testReadExpGolombManyBits(std::array<uint8_t, 16> input) {
      BitCache cache;
      uint8_t index = 0;
      auto val = readExGolombBits(cache, [&](std::byte& out) {
        if (index < input.size()) {
          out = std::bit_cast<std::byte>(input[index++]);
          return true;
        }
        return false;
      });
      return {val.value, val.numBits, cache.value, cache.usedBits};
    }

    // bit-cache is left-adjusted
    static_assert(testFlushFullBytes({0, 0}) == CacheResult{0, 0, 0, 0});
    static_assert(testFlushFullBytes({0, 17}) == CacheResult{0, 16, 0, 1});
    static_assert(testFlushFullBytes({0x123456789, 17}) == CacheResult{0, 16, 0x1234567890000, 1});
    static_assert(testFlushFullBytes({0x0123456789ABCDEF, 17}) == CacheResult{0x0123, 16, 0x456789ABCDEF0000, 1});
    static_assert(testFlushFullBytes({0x0123456789ABCDEF, 31}) == CacheResult{0x012345, 24, 0x6789ABCDEF000000, 7});
    static_assert(testFlushFullBytes({0x0123456789ABCDEF, 61}) ==
                  CacheResult{0x0123456789ABCD, 56, 0xEF00000000000000, 5});
    static_assert(testFlushFullBytes({std::numeric_limits<uintmax_t>::max(), std::numeric_limits<uintmax_t>::digits}) ==
                  CacheResult{std::numeric_limits<uintmax_t>::max(), std::numeric_limits<uintmax_t>::digits, 0, 0});

    static_assert(testWriteBits({0, 0}, {0, 0}) == CacheResult{0, 0, 0, 0});
    static_assert(testWriteBits({0, 7}, {0, 17}) == CacheResult{0, 24, 0, 0});
    static_assert(testWriteBits({0, 7}, {0x12345, 17}) == CacheResult{0x12345, 24, 0, 0});
    static_assert(testWriteBits({0x1200000000000000, 6}, {0x12345, 17}) ==
                  CacheResult{0x1246, 16, 0x8A00000000000000, 7});
    static_assert(testWriteBits({0, 7}, {0x012345678, 31}) == CacheResult{0x48D159, 32, 0xE000000000000000, 6});
    static_assert(testWriteBits({0x1200000000000000, 6}, {0x012345678, 31}) ==
                  CacheResult{0x1291A2B3, 32, 0xC000000000000000, 5});
    static_assert(testWriteBits({0, 7}, {0x0123456789ABCDEF, 61}) ==
                  CacheResult{0x123456789ABCDE, 64, 0xF000000000000000, 4});
    static_assert(testWriteBits({0x1200000000000000, 6}, {0x0123456789ABCDEF, 61}) ==
                  CacheResult{0x122468ACF13579BD, 64, 0xE000000000000000, 3});
    static_assert(testWriteManyBits({0x012345678, 31}).first ==
                  std::array<uint8_t, 16>{0x24, 0x68, 0xAC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00});
    static_assert(testWriteManyBits({0x012345678, 63}).first ==
                  std::array<uint8_t, 16>{0x00, 0x00, 0x00, 0x00, 0x24, 0x68, 0xAC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00});
    static_assert(testWriteManyBits({0x012345678, 79}).first ==
                  std::array<uint8_t, 16>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x68, 0xAC, 0xF0, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00});
    static_assert(testWriteManyBits({0x012345678, 127}).first ==
                  std::array<uint8_t, 16>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24,
                                          0x68, 0xAC, 0xF0});
    static_assert(testWriteManyBits({0xFEDCBA987654321, 63}).first ==
                  std::array<uint8_t, 16>{0x1F, 0xDB, 0x97, 0x53, 0x0E, 0xCA, 0x86, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00});
    static_assert(testWriteManyBits({0xFEDCBA987654321, 127}).first ==
                  std::array<uint8_t, 16>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xDB, 0x97, 0x53, 0x0E,
                                          0xCA, 0x86, 0x42});

    static_assert(testFeedFullByte({0, 0}, 17) == BitCache{0x1100000000000000, 8});
    static_assert(testFeedFullByte({0, 17}, 17) == BitCache{0x88000000000, 25});
    static_assert(testFeedFullByte({0x1234000000000000, 17}, 17) == BitCache{0x1234088000000000, 25});
    static_assert(testFeedFullByte({0x1291A2B300000000, 31}, 17) == BitCache{0x1291A2B322000000, 39});

    static_assert(testReadExpGolombBits({0, 0}, {0, 0}) == CacheResult{0, 0, 0, 0});
    static_assert(testReadExpGolombBits({0, 7}, {0, 17}) == CacheResult{0, 0, 0, 0});
    static_assert(testReadExpGolombBits({0, 7}, {0x12345, 17}) == CacheResult{0x91, 8, 0, 0});
    static_assert(testReadExpGolombBits({0x1200000000000000, 7}, {0, 0}) == CacheResult{9, 4, 0, 0});
    static_assert(testReadExpGolombBits({0, 7}, {0x012345678, 31}) == CacheResult{0x246, 10, 0x8000000000000000, 4});
    static_assert(testReadExpGolombBits({0, 30}, {0x0123456789A, 39}) ==
                  CacheResult{0x123456788, 33, 0xD000000000000000, 5});
    static_assert(testReadExpGolombManyBits({0x00, 0x00, 0x00, 0x00, 0x91, 0xA2, 0xB3, 0xC4, 0xD0, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x00, 0x00}) ==
                  CacheResult{0x123456789, 33, 0xA000000000000000, 7});
    static_assert(testReadExpGolombManyBits({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89,
                                             0xAB, 0xCD, 0xEF, 0x00}) == CacheResult{0x91A2B3C4D5E6F780, 64, 0, 1});
  } // namespace detail
} // namespace serialize
