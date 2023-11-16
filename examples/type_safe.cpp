/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "type_safe.hpp"
#include "deserialize.hpp"
#include "serialize.hpp"
#include "simple.hpp"

#include <cstdlib>
#include <sstream>
#include <string>

int main() {
  std::stringstream ss;

  // Serialization
  std::string object = "So easy";

  serialize::TypeSafeSerializer<serialize::SimpleStreamSerializer> s{serialize::SimpleStreamSerializer{ss}};
  serialize::serialize(s, object);
  s.flush();

  // Deserialization
  // This call fails, since the types do not match
  try {
    serialize::TypeSafeDeserializer<serialize::SimpleStreamDeserializer> d{serialize::SimpleStreamDeserializer{ss}};
    std::ignore = serialize::deserialize<int32_t>(d);
    return EXIT_FAILURE;
  } catch (const std::domain_error&) {
    // Reset stream, so we can read from the beginning
    ss.seekg(0);
    ss.clear();
  }

  // This call succeeds, since the types do match
  serialize::TypeSafeDeserializer<serialize::SimpleStreamDeserializer> d{serialize::SimpleStreamDeserializer{ss}};
  auto secondObject = serialize::deserialize<std::string>(d);

  return object == secondObject ? EXIT_SUCCESS : EXIT_FAILURE;
}
