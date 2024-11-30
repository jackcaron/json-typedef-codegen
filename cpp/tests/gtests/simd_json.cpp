#ifdef USE_SIMD

#include "simd_reader.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

// TODO: a class to hold the parser and document alive for the whole test

static ExpType<Reader::JsonValue>
from_padded_string(simdjson::padded_string& json) {
  ondemand::parser parser;
  auto doc = parser.iterate(json);
  return Reader::simdjson_root_value(doc.get_value());
}

TEST(SIMD_JSON, ignore_empty) {
  auto json_str = R"( )"_padded;

  auto json_val = from_padded_string(json_str);
  EXPECT_FALSE(json_val.has_value());
  EXPECT_EQ(json_val.error().type, JsonErrorTypes::Internal);
}

TEST(SIMD_JSON, ignore_non_array_object) {
  {
    auto json_str = R"( true )"_padded;
    auto json_val = from_padded_string(json_str);
    EXPECT_FALSE(json_val.has_value());
    EXPECT_EQ(json_val.error().type, JsonErrorTypes::WrongType);
  }
  {
    auto json_str = R"( 123 )"_padded;

    auto json_val = from_padded_string(json_str);
    EXPECT_FALSE(json_val.has_value());
    EXPECT_EQ(json_val.error().type, JsonErrorTypes::WrongType);
  }
}

TEST(SIMD_JSON, empty_array) {
  auto json_str = R"( [] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Array);

  auto arr = val.read_array();
  EXPECT_TRUE(arr.has_value());
  bool has_item = false;
  for (auto item : arr.value()) {
    has_item = true;
  }
  EXPECT_FALSE(has_item);
}

TEST(SIMD_JSON, empty_object) {
  auto json_str = R"( {} )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);
}

TEST(SIMD_JSON, array_of_numbers) {
  auto json_str = R"( [1,2,3,4] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Array);

  auto arr = val.read_array();
  EXPECT_TRUE(arr.has_value());
  int64_t exp = 1;
  for (auto item : arr.value()) {
    EXPECT_TRUE(item.has_value());
    auto val = std::move(item.value());
    EXPECT_EQ(val.get_type(), JsonTypes::Number);
    EXPECT_EQ(val.read_i64(), exp);
    ++exp;
  }
  EXPECT_EQ(exp, 5);
}

TEST(SIMD_JSON, array_of_numbers_with_error) {
  auto json_str = R"( [1,2,3 4] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Array);

  auto arr = val.read_array();
  EXPECT_TRUE(arr.has_value());
  int64_t exp = 1;
  for (auto item : arr.value()) {
    if (exp < 4) {
      EXPECT_TRUE(item.has_value());
      auto val = std::move(item.value());
      EXPECT_EQ(val.get_type(), JsonTypes::Number);
      EXPECT_EQ(val.read_i64(), exp);
    } else {
      EXPECT_FALSE(item.has_value());
      auto err = std::move(item.error());
      EXPECT_EQ(err.type, JsonErrorTypes::Internal);
    }
    ++exp;
  }
  EXPECT_EQ(exp, 5);
}

TEST(SIMD_JSON, object_key_val) {
  auto json_str = R"( { "A": 1, "B": 2, "C": 3 } )"_padded;
  constexpr std::array<std::pair<std::string_view, uint64_t>, 3> expectations =
      {{{"A"sv, 1ul}, {"B"sv, 2ul}, {"C"sv, 3ul}}};

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);

  auto obj = val.read_object();
  EXPECT_TRUE(obj.has_value());

  int count = 0;
  auto exp_iter = expectations.begin();
  for (auto kvpair : obj.value()) {
    EXPECT_TRUE(kvpair.has_value());
    auto [key, val] = std::move(kvpair.value());

    EXPECT_EQ(key, exp_iter->first);
    EXPECT_EQ(val.get_type(), JsonTypes::Number);
    EXPECT_EQ(val.read_u64(), exp_iter->second);

    ++exp_iter;
    ++count;
  }
  EXPECT_EQ(count, 3);
}

TEST(SIMD_JSON, object_with_incr_arrays_of_types) {
  auto json_str = R"( {
    "Alice": [1],
    "Bob": [false, true],
    "Chuck": [{}, {}, {}],
    "Dave": [[],[],[],[]],
    "Elle": ["a", "b", "c", "d", "e"]
  } )"_padded;

  constexpr std::array<std::tuple<std::string_view, JsonTypes, int>, 5>
      expectations = {{
          {"Alice"sv, JsonTypes::Number, 1},
          {"Bob"sv, JsonTypes::Bool, 2},
          {"Chuck"sv, JsonTypes::Object, 3},
          {"Dave"sv, JsonTypes::Array, 4},
          {"Elle"sv, JsonTypes::String, 5},
      }};

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);

  auto obj = val.read_object();
  EXPECT_TRUE(obj.has_value());

  int count = 0;
  auto exp_iter = expectations.begin();
  for (auto kvpair : obj.value()) {
    EXPECT_TRUE(kvpair.has_value());
    auto [key, val] = std::move(kvpair.value());

    EXPECT_EQ(key, std::get<0>(*exp_iter));

    EXPECT_EQ(val.get_type(), JsonTypes::Array);
    auto expSubarr = val.read_array();
    EXPECT_TRUE(expSubarr.has_value());

    int arrLen = 0;
    for (auto item : expSubarr.value()) {
      EXPECT_TRUE(item.has_value());
      EXPECT_EQ(item.value().get_type(), std::get<1>(*exp_iter));
      ++arrLen;
    }

    EXPECT_EQ(arrLen, std::get<2>(*exp_iter));

    ++exp_iter;
    ++count;
  }
  EXPECT_EQ(count, 5);
}

#endif // USE_SIMD
