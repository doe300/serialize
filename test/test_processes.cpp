
#include "bit_packing.hpp"
#include "byte_packing.hpp"
#include "deserialize.hpp"
#include "serialize.hpp"
#include "simple.hpp"
#include "type_safe.hpp"

#include "cpptest-main.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct DataContainer {

  int8_t sb;
  uint8_t ub;
  int16_t ss;
  uint16_t us;
  int32_t si;
  uint32_t ui;
  int64_t sl;
  uint64_t ul;

  float f;
  double d;
  long double ld;

  char c;
  wchar_t w;
  char8_t u8;
  char16_t u16;
  char32_t u32;

  bool b;

  std::string s;
  std::vector<std::byte> v;

  constexpr auto operator<=>(const DataContainer& other) const noexcept = default;
};

static const DataContainer VALUES = {
    -3,
    17,
    -1234,
    12345,
    -654321,
    543213440,
    -3751985643563665,
    43759353465875,
    -17.0f,
    4365477356385674763.34563,
    4357357985453435.43568463578623562,
    'a',
    L'β',
    u8'A',
    u'猫',
    U'🍌',
    true,
    "Foo",
    {std::byte{0x07}, std::byte{0x09}, std::byte{0x17}},
};

template <typename D> static DataContainer readDeserialized(std::istream& is) {
  D deserializer{is};
  return serialize::deserialize<DataContainer>(deserializer);
}

static std::filesystem::path getTestFilePath(const std::string& type) {
  return std::filesystem::path{TEST_FILES_PATH} / ("data-" + type + ".bin");
}

class TestSerializers : public Test::Suite {
public:
  TestSerializers() : Suite("TestSerializers") {
    TEST_ADD(TestSerializers::testSimple);
    TEST_ADD(TestSerializers::testBytePacking);
    TEST_ADD(TestSerializers::testBitPacking);
    TEST_ADD(TestSerializers::testTypeSafeSimple);
    TEST_ADD(TestSerializers::testTypeSafeBytePacking);
    TEST_ADD(TestSerializers::testTypeSafeBitPacking);
  }

  void testSimple() {
    auto path = getTestFilePath("simple");
    std::ofstream fos{path};
    serialize::SimpleStreamSerializer s{fos};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }

  void testBytePacking() {
    auto path = getTestFilePath("byte-packing");
    std::ofstream fos{path};
    serialize::BytePackingSinkSerializer s{fos};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }

  void testBitPacking() {
    auto path = getTestFilePath("bit-packing");
    std::ofstream fos{path};
    serialize::BitPackingSinkSerializer s{fos};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }

  void testTypeSafeSimple() {
    auto path = getTestFilePath("type-safe-simple");
    std::ofstream fos{path};
    serialize::TypeSafeSerializer<serialize::SimpleStreamSerializer> s{serialize::SimpleStreamSerializer{fos}};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }

  void testTypeSafeBytePacking() {
    auto path = getTestFilePath("type-safe-byte-packing");
    std::ofstream fos{path};
    serialize::TypeSafeSerializer<serialize::BytePackingSinkSerializer> s{serialize::BytePackingSinkSerializer{fos}};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }

  void testTypeSafeBitPacking() {
    auto path = getTestFilePath("type-safe-bit-packing");
    std::ofstream fos{path};
    serialize::TypeSafeSerializer<serialize::BitPackingSinkSerializer> s{serialize::BitPackingSinkSerializer{fos}};
    serialize::serialize(s, VALUES);
    s.flush();
    testAssert(!!fos);
  }
};

class TestDeserializers : public Test::Suite {
public:
  TestDeserializers() : Suite("TestDeserializers") {
    TEST_ADD(TestDeserializers::testSimple);
    TEST_ADD(TestDeserializers::testBytePacking);
    TEST_ADD(TestDeserializers::testBitPacking);
    TEST_ADD(TestDeserializers::testTypeSafeSimple);
    TEST_ADD(TestDeserializers::testTypeSafeBytePacking);
    TEST_ADD(TestDeserializers::testTypeSafeBitPacking);
  }

  void testSimple() {
    auto path = getTestFilePath("simple");
    std::ifstream fis{path};
    serialize::SimpleStreamDeserializer d{fis};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

  void testBytePacking() {
    auto path = getTestFilePath("byte-packing");
    std::ifstream fis{path};
    serialize::BytePackingSourceDeserializer d{fis};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

  void testBitPacking() {
    auto path = getTestFilePath("bit-packing");
    std::ifstream fis{path};
    serialize::BitPackingSourceDeserializer d{fis};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

  void testTypeSafeSimple() {
    auto path = getTestFilePath("type-safe-simple");
    std::ifstream fis{path};
    serialize::TypeSafeDeserializer<serialize::SimpleStreamDeserializer> d{serialize::SimpleStreamDeserializer{fis}};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

  void testTypeSafeBytePacking() {
    auto path = getTestFilePath("type-safe-byte-packing");
    std::ifstream fis{path};
    serialize::TypeSafeDeserializer<serialize::BytePackingSourceDeserializer> d{
        serialize::BytePackingSourceDeserializer{fis}};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

  void testTypeSafeBitPacking() {
    auto path = getTestFilePath("type-safe-bit-packing");
    std::ifstream fis{path};
    serialize::TypeSafeDeserializer<serialize::BitPackingSourceDeserializer> d{
        serialize::BitPackingSourceDeserializer{fis}};
    checkValue(serialize::deserialize<DataContainer>(d));
    testAssert(!!fis);
  }

private:
  void checkValue(DataContainer&& result) {
    if (result != VALUES) {
      testAssertEquals(VALUES.sb, result.sb);
      testAssertEquals(VALUES.ub, result.ub);
      testAssertEquals(VALUES.ss, result.ss);
      testAssertEquals(VALUES.us, result.us);
      testAssertEquals(VALUES.si, result.si);
      testAssertEquals(VALUES.ui, result.ui);
      testAssertEquals(VALUES.sl, result.sl);
      testAssertEquals(VALUES.ul, result.ul);

      testAssertEquals(VALUES.f, result.f);
      testAssertEquals(VALUES.d, result.d);
      testAssertEquals(VALUES.ld, result.ld);

      testAssertEquals(VALUES.c, result.c);
      testAssertEquals(VALUES.w, result.c);
      testAssertEquals(VALUES.u8, result.u8);
      testAssertEquals(VALUES.u16, result.u16);
      testAssertEquals(VALUES.u32, result.u32);

      testAssertEquals(VALUES.b, result.b);

      testAssertEquals(VALUES.s, result.s);
      testAssertEquals(VALUES.v, result.v);
    }
  }
};

int main(int argc, char** argv) {

  Test::registerSuite(Test::newInstance<TestSerializers>, "serialize", "", Test::RegistrationFlags::OMIT_FROM_DEFAULT);
  Test::registerSuite(Test::newInstance<TestDeserializers>, "deserialize", "",
                      Test::RegistrationFlags::OMIT_FROM_DEFAULT);

  return Test::runSuites(argc, argv);
}
