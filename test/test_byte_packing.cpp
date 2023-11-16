/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "byte_packing.hpp"

#include "test_base.hpp"

using namespace serialize;

class TestBytePackingSerialization
    : public SerializationTestBase<BytePackingSinkSerializer, BytePackingSourceDeserializer> {
public:
  TestBytePackingSerialization() : SerializationTestBase("BytePackingSerialization") {}

  std::tuple<BytePackingSinkSerializer, BytePackingSourceDeserializer>
  createSerializerAndDeserializer(std::stringstream& data) override {
    return std::make_tuple(BytePackingSinkSerializer{data}, BytePackingSourceDeserializer{data});
  }
};

void registerBytePackingTests() {
  Test::registerSuite(Test::newInstance<TestBytePackingSerialization>, "byte-packing");
}
