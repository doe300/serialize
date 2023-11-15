
#include "bit_packing.hpp"

#include "test_base.hpp"

using namespace serialize;

class TestBitPackingSerialization
    : public SerializationTestBase<BitPackingSinkSerializer, BitPackingSourceDeserializer> {
public:
  TestBitPackingSerialization() : SerializationTestBase("BitPackingSerialization") {}

  std::tuple<BitPackingSinkSerializer, BitPackingSourceDeserializer>
  createSerializerAndDeserializer(std::stringstream& data) override {
    return std::make_tuple(BitPackingSinkSerializer{data}, BitPackingSourceDeserializer{data});
  }
};

void registerBitPackingTests() { Test::registerSuite(Test::newInstance<TestBitPackingSerialization>, "bit-packing"); }
