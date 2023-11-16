/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "type_safe.hpp"

#include "bit_packing.hpp"
#include "byte_packing.hpp"
#include "simple.hpp"

#include "cpptest.h"
#include "test_base.hpp"

#include <stdexcept>

using namespace serialize;

template <Serializer S, Deserializable D>
class TestTypeSafeSerialization : public SerializationTestBase<TypeSafeSerializer<S>, TypeSafeDeserializer<D>> {
public:
  TestTypeSafeSerialization(std::string name)
      : SerializationTestBase<TypeSafeSerializer<S>, TypeSafeDeserializer<D>>::SerializationTestBase(std::move(name)) {
    TEST_ADD(TestTypeSafeSerialization::testTypeViolation);
  }

  std::tuple<TypeSafeSerializer<S>, TypeSafeDeserializer<D>>
  createSerializerAndDeserializer(std::stringstream& data) override {
    return std::make_tuple(TypeSafeSerializer<S>{S{data}}, TypeSafeDeserializer<D>{D{data}});
  }

  void testTypeViolation() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, float{17.0});
    serializer.flush();
    this->template testThrows<std::domain_error>([&] { serialize::deserialize<int32_t>(deserializer); });
  }
};

void registerTypeSafeTests() {
  Test::registerSuite(
      []() {
        return new TestTypeSafeSerialization<SimpleStreamSerializer, SimpleStreamDeserializer>{
            "TypeSafeSimpleSerialization"};
      },
      "type-safe-simple");
  Test::registerSuite(
      []() {
        return new TestTypeSafeSerialization<BytePackingSinkSerializer, BytePackingSourceDeserializer>{
            "TypeSafeBytePackingSerialization"};
      },
      "type-safe-byte-packing");
  Test::registerSuite(
      []() {
        return new TestTypeSafeSerialization<BitPackingSinkSerializer, BitPackingSourceDeserializer>{
            "TypeSafeBitPackingSerialization"};
      },
      "type-safe-bit-packing");
}
