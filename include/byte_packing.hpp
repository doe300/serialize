#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"

#include <functional>
#include <iostream>

namespace serialize {

  /**
   * Serializer wrapping a std::ostream or byte sink function applying byte-lever compression via a custom coding.
   */
  class BytePackingSinkSerializer {
  public:
    using SinkByte = std::function<void(std::byte)>;

    explicit BytePackingSinkSerializer(SinkByte&& sink) : sink(std::move(sink)) {}
    explicit BytePackingSinkSerializer(std::ostream& os);

    template <typename T> std::enable_if_t<std::is_integral_v<T>> write(T val) {
      using MaxIntType = std::conditional_t<std::is_signed_v<T>, intmax_t, uintmax_t>;
      write(MaxIntType{val});
    }

    void write(float val);
    void write(double val);
    void write(long double val);

    void write(intmax_t val);
    void write(uintmax_t val);

    void flush() {}

  private:
    SinkByte sink;
  };

  /**
   * Deserializer wrapping a std::istream or byte source function applying byte-level decompression using a custom
   * coding.
   */
  class BytePackingSourceDeserializer {
  public:
    using SourceByte = std::function<bool(std::byte&)>;

    explicit BytePackingSourceDeserializer(SourceByte&& source) : source(std::move(source)) {}
    explicit BytePackingSourceDeserializer(std::istream& is);

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
  };

} // namespace serialize
