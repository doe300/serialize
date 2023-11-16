/*
 * Compressing Serializer and Deserializer using Exponential-Golomb coding.
 *
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */
#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"

#include <functional>
#include <iostream>

namespace serialize {

  struct BitCache {
    uintmax_t value = 0;
    uint8_t usedBits = 0;

    constexpr auto operator<=>(const BitCache&) const noexcept = default;
  };

  /**
   * Serializer wrapping a std::ostream or byte sink function compressing integral values via Exponential-Golomb coding.
   *
   * NOTE: This serializer requires proper usage of the #flush() function.
   */
  class BitPackingSinkSerializer {
  public:
    using SinkByte = std::function<void(std::byte)>;

    explicit BitPackingSinkSerializer(SinkByte&& sink) : sink(std::move(sink)) {}
    explicit BitPackingSinkSerializer(std::ostream& os);

    template <typename T> std::enable_if_t<std::is_integral_v<T>> write(T val) {
      using MaxIntType = std::conditional_t<std::is_signed_v<T>, intmax_t, uintmax_t>;
      write(MaxIntType{val});
    }

    void write(float val);
    void write(double val);
    void write(long double val);

    void write(intmax_t val);
    void write(uintmax_t val);

    void flush();

  private:
    SinkByte sink;
    BitCache cache;
  };

  /**
   * Deserializer wrapping a std::istream or byte source function decompressing integral values via Exponential-Golomb
   * coding.
   */
  class BitPackingSourceDeserializer {
  public:
    using SourceByte = std::function<bool(std::byte&)>;

    explicit BitPackingSourceDeserializer(SourceByte&& source) : source(std::move(source)) {}
    explicit BitPackingSourceDeserializer(std::istream& is);

    template <typename T> std::enable_if_t<std::is_integral_v<T>> read(T& val) {
      using MaxIntType = std::conditional_t<std::is_signed_v<T>, intmax_t, uintmax_t>;
      MaxIntType tmp = 0;
      read(tmp);
      val = static_cast<T>(tmp);
    }

    void read(float& val);
    void read(double& val);
    void read(long double& val);

    void read(intmax_t& val);
    void read(uintmax_t& val);

  private:
    SourceByte source;
    BitCache cache;
  };

} // namespace serialize
