
#include "simple.hpp"

#include "test_base.hpp"

using namespace serialize;

class TestSimpleSerialization : public SerializationTestBase<SimpleStreamSerializer, SimpleStreamDeserializer> {
public:
  TestSimpleSerialization() : SerializationTestBase("SimpleSerialization") {}

  std::tuple<SimpleStreamSerializer, SimpleStreamDeserializer>
  createSerializerAndDeserializer(std::stringstream& data) override {
    return std::make_tuple(SimpleStreamSerializer{data}, SimpleStreamDeserializer{data});
  }
};

void registerSimpleTests() { Test::registerSuite(Test::newInstance<TestSimpleSerialization>, "simple"); }
