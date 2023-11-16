/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "simple.hpp"

namespace serialize {

  static_assert(Serializer<SimpleStreamSerializer>);
  static_assert(ByteSerializer<SimpleStreamSerializer>);
  static_assert(Deserializer<SimpleStreamDeserializer>);

} // namespace serialize
