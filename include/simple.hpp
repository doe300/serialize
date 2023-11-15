#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"

#include <iostream>

namespace serialize {

  /**
   * Simple Serializer wrapping a std::ostream.
   */
  class SimpleStreamSerializer {
  public:
    explicit SimpleStreamSerializer(std::ostream& os) : out(os) {}

    template <typename T> std::enable_if_t<std::is_fundamental_v<T>> write(T val) {
      out.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }

    void write(std::size_t numElements, std::span<const std::byte> data) {
      write(numElements);
      out.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    void flush() {}

  private:
    std::ostream& out;
  };

  /**
   * Simple Deserializer wrapping a std::istream.
   */
  class SimpleStreamDeserializer {
  public:
    explicit SimpleStreamDeserializer(std::istream& is) : in(is) {}

    template <typename T> std::enable_if_t<std::is_fundamental_v<T>> read(T& val) {
      if (!in.read(reinterpret_cast<char*>(&val), sizeof(T))) {
        detail::throwOnEof();
      }
    }

  private:
    std::istream& in;
  };

} // namespace serialize
