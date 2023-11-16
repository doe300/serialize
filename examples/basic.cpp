/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

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

  serialize::SimpleStreamSerializer s{ss};
  serialize::serialize(s, object);
  s.flush();

  // Deserialization
  serialize::SimpleStreamDeserializer d{ss};
  auto secondObject = serialize::deserialize<std::string>(d);

  return object == secondObject ? EXIT_SUCCESS : EXIT_FAILURE;
}
