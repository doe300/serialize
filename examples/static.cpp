/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "byte_packing.hpp"
#include "deserialize.hpp"
#include "serialize.hpp"
#include "traits.hpp"

#include <cstdlib>
#include <sstream>
#include <string>

// Custom type with custom static (de-)serialization functions
class MyType {
public:
  MyType() = default;
  MyType(uint32_t val) : storage(val) {}

  template <serialize::Serializer S> static void serialize(S& serializer, const MyType& object) {
    serialize::serialize(serializer, std::any_cast<uint32_t>(object.storage));
  }

  template <serialize::Deserializer D> static void deserialize(D& deserializer, MyType& object) {
    object.storage = serialize::deserialize<uint32_t>(deserializer);
  }

  bool operator==(const MyType& other) const noexcept {
    return storage.type() == typeid(uint32_t) && other.storage.type() == typeid(uint32_t) &&
           std::any_cast<uint32_t>(storage) == std::any_cast<uint32_t>(other.storage);
  }

private:
  std::any storage;
};

int main() {
  std::stringstream ss;

  // Serialization
  const MyType input{42};

  serialize::BytePackingSinkSerializer s{ss};
  serialize::serialize(s, input);
  s.flush();

  // Deserialization
  serialize::BytePackingSourceDeserializer d{ss};
  auto output = serialize::deserialize<MyType>(d);

  return input == output ? EXIT_SUCCESS : EXIT_FAILURE;
}
