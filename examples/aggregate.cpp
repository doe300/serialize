/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "deserialize.hpp"
#include "serialize.hpp"
#include "bit_packing.hpp"

#include <compare>
#include <cstdlib>
#include <sstream>
#include <string>

// Simple aggregate type, can be (de-)serialized without any specific code, as long as all members are
// (de-)serializable.
struct MyAggregate {
  uint32_t u;
  float f;
  std::string s;
  std::variant<int64_t, double> v;

  friend auto operator<=>(const MyAggregate&, const MyAggregate&) = default;
};

int main() {
  std::stringstream ss;

  // Serialization
  const MyAggregate input = {42, -17.0f, "Foo", 123};

  serialize::BitPackingSinkSerializer s{ss};
  serialize::serialize(s, input);
  s.flush();

  // Deserialization
  serialize::BitPackingSourceDeserializer d{ss};
  auto output = serialize::deserialize<MyAggregate>(d);

  return input == output ? EXIT_SUCCESS : EXIT_FAILURE;
}
