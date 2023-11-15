
#include "byte_packing.hpp"

#include <array>
#include <bit>

namespace serialize {

  static_assert(Serializer<BytePackingSinkSerializer>);
  static_assert(!ByteSerializer<BytePackingSinkSerializer>);
  static_assert(Deserializer<BytePackingSourceDeserializer>);

  static constexpr uint8_t BYTE_VALUE_MASK = 0x7F;
  static constexpr uint8_t BYTE_CONTINUATION_FLAG = 0x80;
  static constexpr uint8_t BYTE_CONTINUATION_OFFSET = 7;
  static_assert((BYTE_VALUE_MASK & BYTE_CONTINUATION_FLAG) == 0);
  static_assert((BYTE_VALUE_MASK | BYTE_CONTINUATION_FLAG) == 0xFF);
  static_assert(BYTE_CONTINUATION_FLAG >> BYTE_CONTINUATION_OFFSET == 1);

  BytePackingSinkSerializer::BytePackingSinkSerializer(std::ostream& os)
      : BytePackingSinkSerializer([&os](std::byte byte) {
          char tmp = std::bit_cast<char>(byte);
          os.write(&tmp, 1);
        }) {}

  void BytePackingSinkSerializer::write(float val) { write(std::bit_cast<uint32_t>(val)); }
  void BytePackingSinkSerializer::write(double val) { write(std::bit_cast<uint64_t>(val)); }

  void BytePackingSinkSerializer::write(long double val) {
    static_assert(sizeof(long double) % sizeof(uint64_t) == 0);
    auto data = std::bit_cast<std::array<uint64_t, sizeof(long double) / sizeof(uint64_t)>>(val);
    for (auto& entry : data) {
      write(entry);
    }
  }

  void BytePackingSinkSerializer::write(intmax_t val) { write(std::bit_cast<uintmax_t>(val)); }

  void BytePackingSinkSerializer::write(uintmax_t val) {
    // write as few bytes as necessary to store the value.
    // every written byte contains 1 "more bytes" bit and 7 data bits, resulting in at most 10 byte per value for 64-bit
    // values. values are stored in little-endian order.
    if (!val) {
      sink(std::byte{0});
      return;
    }

    while (val) {
      auto current = val & BYTE_VALUE_MASK;
      val >>= BYTE_CONTINUATION_OFFSET;

      sink(static_cast<std::byte>(current | (val ? BYTE_CONTINUATION_FLAG : 0x00)));
    }
  }

  BytePackingSourceDeserializer::BytePackingSourceDeserializer(std::istream& is)
      : BytePackingSourceDeserializer([&is](std::byte& out) {
          char tmp = 0;
          if (is.read(&tmp, 1)) {
            out = std::bit_cast<std::byte>(tmp);
            return true;
          }
          return false;
        }) {}

  void BytePackingSourceDeserializer::read(float& val) {
    uint32_t tmp = 0;
    read(tmp);
    val = std::bit_cast<float>(tmp);
  }

  void BytePackingSourceDeserializer::read(double& val) {
    uint64_t tmp = 0;
    read(tmp);
    val = std::bit_cast<double>(tmp);
  }

  void BytePackingSourceDeserializer::read(long double& val) {
    static_assert(sizeof(long double) % sizeof(uint64_t) == 0);
    std::array<uint64_t, sizeof(long double) / sizeof(uint64_t)> data{};
    for (auto& entry : data) {
      read(entry);
    }
    val = std::bit_cast<long double>(data);
  }

  void BytePackingSourceDeserializer::read(intmax_t& val) {
    uintmax_t tmp = 0;
    read(tmp);
    val = std::bit_cast<intmax_t>(tmp);
  }

  void BytePackingSourceDeserializer::read(uintmax_t& val) {
    val = 0;
    uint32_t offset = 0;
    std::byte byte{};
    while (source(byte)) {
      bool hasMore = std::bit_cast<uint8_t>(byte) & BYTE_CONTINUATION_FLAG;
      uintmax_t current = std::bit_cast<uint8_t>(byte) & BYTE_VALUE_MASK;
      val |= current << offset;
      offset += BYTE_CONTINUATION_OFFSET;

      if (!hasMore) {
        return;
      }
    }
    detail::throwOnEof();
  }

} // namespace serialize
