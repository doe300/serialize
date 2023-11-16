/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "deserialize.hpp"
#include "serialize.hpp"

#include <array>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace serialize {
  static_assert(SerializableRawData<int[5]>);
  static_assert(SerializableRawData<std::array<int, 5>>);
  static_assert(SerializableRawData<std::span<int>>);
  static_assert(SerializableRawData<std::string>);
  static_assert(SerializableRawData<std::string_view>);
  static_assert(SerializableRawData<std::vector<int>>);

  static_assert(SerializableContainer<std::string[5]>);
  static_assert(SerializableContainer<std::array<std::string, 5>>);
  static_assert(SerializableContainer<std::map<int, int>>);
  static_assert(SerializableContainer<std::map<std::string, std::string>>);
  static_assert(SerializableContainer<std::set<int>>);
  static_assert(SerializableContainer<std::set<std::string>>);
  static_assert(SerializableContainer<std::span<std::string>>);
  static_assert(SerializableContainer<std::unordered_map<int, int>>);
  static_assert(SerializableContainer<std::unordered_map<std::string, std::string>>);
  static_assert(SerializableContainer<std::unordered_set<int>>);
  static_assert(SerializableContainer<std::unordered_set<std::string>>);
  static_assert(SerializableContainer<std::vector<std::string>>);

  static_assert(DeserializableGrowableContainer<std::string>);
  static_assert(DeserializableGrowableContainer<std::map<int, int>>);
  static_assert(DeserializableGrowableContainer<std::map<std::string, std::string>>);
  static_assert(DeserializableGrowableContainer<std::set<int>>);
  static_assert(DeserializableGrowableContainer<std::set<std::string>>);
  static_assert(DeserializableGrowableContainer<std::unordered_map<int, int>>);
  static_assert(DeserializableGrowableContainer<std::unordered_map<std::string, std::string>>);
  static_assert(DeserializableGrowableContainer<std::unordered_set<int>>);
  static_assert(DeserializableGrowableContainer<std::unordered_set<std::string>>);
  static_assert(DeserializableGrowableContainer<std::vector<std::string>>);
} // namespace serialize
