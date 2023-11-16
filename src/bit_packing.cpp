/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "bit_packing.hpp"

#include "bit_helpers.hpp"

#include <array>
#include <bit>

namespace serialize {

  static_assert(Serializer<BitPackingSinkSerializer>);
  static_assert(!ByteSerializer<BitPackingSinkSerializer>);
  static_assert(Deserializer<BitPackingSourceDeserializer>);

  BitPackingSinkSerializer::BitPackingSinkSerializer(std::ostream& os)
      : BitPackingSinkSerializer([&os](std::byte byte) {
          char tmp = std::bit_cast<char>(byte);
          os.write(&tmp, 1);
        }) {}

  void BitPackingSinkSerializer::write(float val) {
    // Floating-point values tend to have some of the higher bits set (due to the exponent being located in the higher
    // bits) more often than having the lower bits set (e.g. the lower bits of the mantissa are often zero, esp. for
    // "round" floating-point values like powers of two).
    // => Thus, invert the values before writing
    write(reverseBits<uint32_t>(std::bit_cast<uint32_t>(val)));
  }

  void BitPackingSinkSerializer::write(double val) { write(reverseBits<uint64_t>(std::bit_cast<uint64_t>(val))); }

  void BitPackingSinkSerializer::write(long double val) {
    static_assert(sizeof(long double) % sizeof(uint64_t) == 0);
    auto data = std::bit_cast<std::array<uint64_t, sizeof(long double) / sizeof(uint64_t)>>(val);
    for (auto& entry : data) {
      write(reverseBits<uint64_t>(entry));
    }
  }

  void BitPackingSinkSerializer::write(intmax_t val) { return writeBits(cache, sink, encodeSignedExpGolomb(val)); }

  void BitPackingSinkSerializer::write(uintmax_t val) {
    // write as Exp-Golomb
    writeBits(cache, sink, encodeExpGolomb(val));
  }

  void BitPackingSinkSerializer::flush() {
    flushFullBytes(cache, sink);
    // append trailing zeroes until we fill the last byte
    while (cache.usedBits) {
      cache.usedBits += 1;
      flushFullBytes(cache, sink);
    }
  }

  BitPackingSourceDeserializer::BitPackingSourceDeserializer(std::istream& is)
      : BitPackingSourceDeserializer([&is](std::byte& out) {
          char tmp = 0;
          if (is.read(&tmp, 1)) {
            out = std::bit_cast<std::byte>(tmp);
            return true;
          }
          return false;
        }) {}

  void BitPackingSourceDeserializer::read(float& val) {
    uint32_t tmp = 0;
    read(tmp);
    // Revert bits back, see writer comment
    val = std::bit_cast<float>(reverseBits<uint32_t>(tmp));
  }

  void BitPackingSourceDeserializer::read(double& val) {
    uint64_t tmp = 0;
    read(tmp);
    val = std::bit_cast<double>(reverseBits<uint64_t>(tmp));
  }

  void BitPackingSourceDeserializer::read(long double& val) {
    static_assert(sizeof(long double) % sizeof(uint64_t) == 0);
    std::array<uint64_t, sizeof(long double) / sizeof(uint64_t)> data{};
    for (auto& entry : data) {
      read(entry);
      entry = reverseBits<uint64_t>(entry);
    }
    val = std::bit_cast<long double>(data);
  }

  void BitPackingSourceDeserializer::read(intmax_t& val) {
    if (auto encoded = readExGolombBits(cache, source); encoded.numBits) {
      val = decodeSignedExpGolomb(encoded.value);
    } else {
      detail::throwOnEof();
    }
  }
  void BitPackingSourceDeserializer::read(uintmax_t& val) {
    if (auto encoded = readExGolombBits(cache, source); encoded.numBits) {
      val = decodeExpGolomb(encoded.value);
    } else {
      detail::throwOnEof();
    }
  }

} // namespace serialize
