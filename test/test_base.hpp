/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */
#pragma once

#include "deserialize.hpp"
#include "serialize.hpp"
#include "traits.hpp"

#include "cpptest-main.h"

#include <bitset>
#include <compare>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

static_assert(serialize::Serializable<const void*>);
static_assert(!serialize::Deserializable<const void*>);

struct FundamentalTypes {

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

  // void* ptr;
  bool b;

  constexpr auto operator<=>(const FundamentalTypes& other) const noexcept = default;
};

static_assert(std::is_trivial_v<FundamentalTypes>);
static_assert(serialize::Serializable<FundamentalTypes>);
static_assert(serialize::Deserializable<FundamentalTypes>);

struct UserDefinedMemberSerialization {
  UserDefinedMemberSerialization() = default;
  UserDefinedMemberSerialization(const UserDefinedMemberSerialization&) = delete;
  UserDefinedMemberSerialization(UserDefinedMemberSerialization&& other) noexcept
      : storage(std::move(other.storage)), reference(storage) {}
  ~UserDefinedMemberSerialization() noexcept = default;

  UserDefinedMemberSerialization& operator=(const UserDefinedMemberSerialization&) = delete;
  UserDefinedMemberSerialization& operator=(UserDefinedMemberSerialization&& other) noexcept {
    storage = std::move(other.storage);
    reference = storage;
    return *this;
  }

  template <serialize::Serializer S> void serialize(S& serializer) const { serialize::serialize(serializer, storage); }

  template <serialize::Deserializer D> void deserialize(D& deserializer) {
    serialize::deserializeInto(deserializer, storage);
    reference = storage;
  }

  std::string storage;
  std::string_view reference;
};
static_assert(!serialize::Deserializable<std::string_view>);
static_assert(serialize::Serializable<UserDefinedMemberSerialization>);
static_assert(serialize::Deserializable<UserDefinedMemberSerialization>);

struct UserDefinedStaticMemberSerialization {
  UserDefinedStaticMemberSerialization() = default;
  UserDefinedStaticMemberSerialization(const UserDefinedStaticMemberSerialization&) = delete;
  UserDefinedStaticMemberSerialization(UserDefinedStaticMemberSerialization&& other) noexcept
      : storage(std::move(other.storage)), reference(storage) {}
  ~UserDefinedStaticMemberSerialization() noexcept = default;

  UserDefinedStaticMemberSerialization& operator=(const UserDefinedStaticMemberSerialization&) = delete;
  UserDefinedStaticMemberSerialization& operator=(UserDefinedStaticMemberSerialization&& other) noexcept {
    storage = std::move(other.storage);
    reference = storage;
    return *this;
  }

  template <serialize::Serializer S>
  static void serialize(S& serializer, const UserDefinedStaticMemberSerialization& val) {
    serialize::serialize(serializer, val.storage);
  }

  template <serialize::Deserializer D>
  static void deserialize(D& deserializer, UserDefinedStaticMemberSerialization& val) {
    serialize::deserializeInto(deserializer, val.storage);
    val.reference = val.storage;
  }

  std::string storage;
  std::string_view reference;
};
static_assert(!serialize::Deserializable<std::string_view>);
static_assert(serialize::Serializable<UserDefinedStaticMemberSerialization>);
static_assert(serialize::Deserializable<UserDefinedStaticMemberSerialization>);

template <typename Serializer, typename Deserializer> class SerializationTestBase : public Test::Suite {
protected:
  SerializationTestBase(std::string name) : Suite(std::move(name)) {
    TEST_ADD(SerializationTestBase::testArrayOfFloats);
    TEST_ADD(SerializationTestBase::testVectorOfIntegers);
    TEST_ADD(SerializationTestBase::testVectorOfStrings);
    TEST_ADD(SerializationTestBase::testMap);
    TEST_ADD(SerializationTestBase::testTrivialUserDefinedType);
    TEST_ADD(SerializationTestBase::testMemberSerializationFunctions);
    TEST_ADD(SerializationTestBase::testStaticMemberSerializationFunctions);
    TEST_ADD(SerializationTestBase::testSpecialStdTypes);
    TEST_ADD(SerializationTestBase::testMultiValue);
    TEST_ADD(SerializationTestBase::testThrowOnEof);
    TEST_ADD(SerializationTestBase::reportBufferSizes);
  }

  void testArrayOfFloats() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, SOME_ARRAY);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto result = serialize::deserialize<std::remove_cv_t<decltype(SOME_ARRAY)>>(deserializer);
    testAssertEquals(SOME_ARRAY, result);
    for (std::size_t i = 0; i < SOME_ARRAY.size(); ++i) {
      if (SOME_ARRAY.at(i) != result.at(i)) {
        testAssertEquals("", "For index: " + std::to_string(i));
      }
    }

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testVectorOfIntegers() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, SOME_NUMBERS);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto result = serialize::deserialize<std::remove_cv_t<decltype(SOME_NUMBERS)>>(deserializer);
    testAssertEquals(SOME_NUMBERS, result);
    for (std::size_t i = 0; i < SOME_NUMBERS.size(); ++i) {
      if (SOME_NUMBERS.at(i) != result.at(i)) {
        testAssertEquals("", "For index: " + std::to_string(i));
      }
    }

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testVectorOfStrings() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, SOME_STRINGS);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto result = serialize::deserialize<std::remove_cv_t<decltype(SOME_STRINGS)>>(deserializer);
    testAssertEquals(SOME_STRINGS, result);
    for (std::size_t i = 0; i < SOME_STRINGS.size(); ++i) {
      if (SOME_STRINGS.at(i) != result.at(i)) {
        testAssertEquals("", "For index: " + std::to_string(i));
      }
    }

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testMap() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, SOME_MAP);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto result = serialize::deserialize<std::remove_cv_t<decltype(SOME_MAP)>>(deserializer);
    testAssertEquals(SOME_MAP.size(), result.size());
    auto refIt = SOME_MAP.begin();
    auto resIt = result.begin();
    for (; refIt != SOME_MAP.end() && resIt != result.end(); ++refIt, ++resIt) {
      testAssertEquals(refIt->first, resIt->first);
      testAssertEquals(refIt->second, resIt->second);
      if (refIt->second != resIt->second) {
        testAssertEquals("", "For entry: " + std::to_string(refIt->first));
      }
    }

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testTrivialUserDefinedType() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, FUNDAMENTAL_TYPES);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto result = serialize::deserialize<std::remove_cv_t<decltype(FUNDAMENTAL_TYPES)>>(deserializer);

    if (result != FUNDAMENTAL_TYPES) {
      testAssertEquals(FUNDAMENTAL_TYPES.sb, result.sb);
      testAssertEquals(FUNDAMENTAL_TYPES.ub, result.ub);
      testAssertEquals(FUNDAMENTAL_TYPES.ss, result.ss);
      testAssertEquals(FUNDAMENTAL_TYPES.us, result.us);
      testAssertEquals(FUNDAMENTAL_TYPES.si, result.si);
      testAssertEquals(FUNDAMENTAL_TYPES.ui, result.ui);
      testAssertEquals(FUNDAMENTAL_TYPES.sl, result.sl);
      testAssertEquals(FUNDAMENTAL_TYPES.ul, result.ul);

      testAssertEquals(FUNDAMENTAL_TYPES.f, result.f);
      testAssertEquals(FUNDAMENTAL_TYPES.d, result.d);
      testAssertEquals(FUNDAMENTAL_TYPES.ld, result.ld);

      testAssertEquals(FUNDAMENTAL_TYPES.c, result.c);
      testAssertEquals(FUNDAMENTAL_TYPES.w, result.c);
      testAssertEquals(FUNDAMENTAL_TYPES.u8, result.u8);
      testAssertEquals(FUNDAMENTAL_TYPES.u16, result.u16);
      testAssertEquals(FUNDAMENTAL_TYPES.u32, result.u32);

      // testAssertEquals(FUNDAMENTAL_TYPES.ptr, result.ptr);
      testAssertEquals(FUNDAMENTAL_TYPES.b, result.b);
    }

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testMemberSerializationFunctions() {
    UserDefinedMemberSerialization input{};
    input.storage = "Foo bar";
    input.reference = input.storage;
    testAssertEquals(input.storage.data(), input.reference.data());

    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, input);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto output = serialize::deserialize<decltype(input)>(deserializer);

    testAssertEquals(input.storage, output.storage);
    testAssertEquals(output.storage.data(), output.reference.data());

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testStaticMemberSerializationFunctions() {
    UserDefinedStaticMemberSerialization input{};
    input.storage = "Fuz row tadaa";
    input.reference = input.storage;
    testAssertEquals(input.storage.data(), input.reference.data());

    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);
    serialize::serialize(serializer, input);
    serializer.flush();
    totalBufferSize += getBufferSize(data);
    auto output = serialize::deserialize<decltype(input)>(deserializer);

    testAssertEquals(input.storage, output.storage);
    testAssertEquals(output.storage.data(), output.reference.data());

    if (hasFailed()) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testSpecialStdTypes() {
    {
      std::atomic<uint16_t> input{17};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::chrono::microseconds input{42};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      auto input = std::chrono::steady_clock::now();
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input.time_since_epoch(), result.time_since_epoch());
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::complex<double> input{17, 4};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::optional<std::string> input{"Foo"};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::optional<std::string> input{};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::unique_ptr<std::string> input{std::make_unique<std::string>("Foo")};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(!!input, !!result);
      testAssertEquals(*input, *result);
      if (*input != *result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::unique_ptr<std::string> input{};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(!!input, !!result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::tuple<int, std::string, double> input{17, "Baz", -42.42};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::variant<double, std::string> input{-42.42};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::bitset<31> input{0b010010101010100101};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::bitset<32> input{0b011010101110010101010100101};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::bitset<33> input{0b0100101010010101010101011110100101};
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }

    {
      std::bitset<267> input{0b010101010101010101010101010010101010100101};
      input.set(176);
      input.set(200);
      input.set(231);
      input.set(265);
      input.set(266);
      std::stringstream data{};
      auto [serializer, deserializer] = createSerializerAndDeserializer(data);
      serialize::serialize(serializer, input);
      serializer.flush();
      totalBufferSize += getBufferSize(data);
      auto result = serialize::deserialize<decltype(input)>(deserializer);
      testAssertEquals(input, result);
      if (input != result) {
        testAssertEquals("", toSerializedDataString(data));
      }
    }
  }

  void testMultiValue() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);

    std::atomic<uint16_t> input0{17};
    std::chrono::microseconds input1{42};
    std::complex<double> input2{17, 4};
    std::optional<std::string> input3{"Foo"};

    serialize::serialize(serializer, input0);
    serialize::serialize(serializer, input1);
    serialize::serialize(serializer, input2);
    serialize::serialize(serializer, input3);
    serializer.flush();

    totalBufferSize += getBufferSize(data);

    auto result0 = serialize::deserialize<decltype(input0)>(deserializer);
    testAssertEquals(input0, result0);
    if (input0 != result0) {
      testAssertEquals("", toSerializedDataString(data));
    }

    auto result1 = serialize::deserialize<decltype(input1)>(deserializer);
    testAssertEquals(input1, result1);
    if (input1 != result1) {
      testAssertEquals("", toSerializedDataString(data));
    }

    auto result2 = serialize::deserialize<decltype(input2)>(deserializer);
    testAssertEquals(input2, result2);
    if (input2 != result2) {
      testAssertEquals("", toSerializedDataString(data));
    }

    auto result3 = serialize::deserialize<decltype(input3)>(deserializer);
    testAssertEquals(input3, result3);
    if (input3 != result3) {
      testAssertEquals("", toSerializedDataString(data));
    }
  }

  void testThrowOnEof() {
    std::stringstream data{};
    auto [serializer, deserializer] = createSerializerAndDeserializer(data);

    // serialize single int
    serialize::serialize(serializer, int32_t{17});

    // try to deserialize string with 17 elements, must throw
    try {
      serialize::deserialize<std::string>(deserializer);
      testFail("No exception thrown!");
    } catch (const std::out_of_range&) {
      // pass
    } catch (const std::domain_error&) {
      // pass (for type-safe variants)
    } catch (const std::exception&) {
      testFail("Unexpected exception thrown!");
    } catch (...) {
      testFail("Non-std::exception thrown!");
    }
  }

  void reportBufferSizes() {
    std::cout << "Total serialization bytes used by '" << getName() << "': " << totalBufferSize << std::endl;
  }

  virtual std::tuple<Serializer, Deserializer> createSerializerAndDeserializer(std::stringstream& data) = 0;

private:
  static std::size_t getBufferSize(std::stringstream& data) { return data.str().size(); }

  static std::string toSerializedDataString(std::stringstream& data) {
    data.seekg(0);
    data.clear();
    std::stringstream out;
    char c = '\0';
    while ((data >> c)) {
      out << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);
    }
    return out.str();
  }

protected:
  inline static const std::vector<int> SOME_NUMBERS = {
      1,
      3,
      17,
      42,
      -113,
      125647,
      -1111,
      42,
      1536,
      466772,
      342455636,
      std::numeric_limits<int>::max(),
      std::numeric_limits<int>::min(),
  };

  inline static const std::vector<std::string> SOME_STRINGS = {
      "Hello", "this", "is",
      "a",     "test", "string which is suddenly much longer, short string optimization and such things, you know!",
  };

  inline static const std::map<int, std::string> SOME_MAP = {{5, "Five"}, {6, "Six"}, {12, "Twelve"}, {17, "Infinite"}};

  inline static const std::array<float, 7> SOME_ARRAY = {656.434, 536.34, -7686867.56, -342342.56,
                                                         23434.0, -54646, 32434.233};

  inline static const FundamentalTypes FUNDAMENTAL_TYPES{
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
      // reinterpret_cast<void*>(123466),
      true,
  };

private:
  std::size_t totalBufferSize = 0;
};
