/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "deserialize.hpp"
#include "serialize.hpp"
#include "traits.hpp"

#include <cstdint>
#include <cstdlib>
#include <deque>
#include <string>

// Custom serializer implementation, storing all written values in a vector.
struct MySerializer {

  template <typename T> void write(T val) { storage.push_back(static_cast<intmax_t>(val)); }

  void flush() {
    // no-op
  }

  std::deque<intmax_t> storage;
};
static_assert(serialize::Serializer<MySerializer>);

// Custom deserializer implementation, reading all values from a vector.
struct MyDeserializer {

  template <typename T> void read(T& out) {
    out = static_cast<T>(storage.front());
    storage.pop_front();
  }

  std::deque<intmax_t> storage;
};
static_assert(serialize::Deserializer<MyDeserializer>);

int main() {

  // Serialization
  const std::string object = "So easy";
  const int32_t val = -42;

  MySerializer s{};
  serialize::serialize(s, object);
  serialize::serialize(s, val);
  s.flush();

  // Deserialization
  MyDeserializer d{std::move(s.storage)};
  auto secondObject = serialize::deserialize<std::string>(d);
  auto secondVal = serialize::deserialize<int32_t>(d);

  return object == secondObject && val == secondVal ? EXIT_SUCCESS : EXIT_FAILURE;
}
