#ifdef USE_SIMD

#include "simd.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

TEST(CLONE_JSON, empty_array) {
  auto json_str = R"( [] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto exp_clone = json_val.value().clone();
  EXPECT_TRUE(exp_clone.has_value());

  auto exp_arr = exp_clone.value();
  EXPECT_TRUE(exp_arr.get_type() == JsonTypes::Array);

  auto cloned_array = exp_arr.read_array().value();
  EXPECT_TRUE(cloned_array.empty());
}

TEST(CLONE_JSON, array_of_numbers) {
  auto json_str = R"( [1,2,3,4] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto exp_clone = json_val.value().clone();
  EXPECT_TRUE(exp_clone.has_value());

  auto exp_arr = exp_clone.value();
  EXPECT_TRUE(exp_arr.get_type() == JsonTypes::Array);

  auto cloned_array = exp_arr.read_array().value();
  EXPECT_EQ(cloned_array.size(), 4);

  int64_t exp = 1;
  for (auto item : cloned_array) {
    EXPECT_EQ(item.get_type(), JsonTypes::Number);
    EXPECT_EQ(item.read_i64(), exp);
    ++exp;
  }
  EXPECT_EQ(exp, 5);
}

TEST(CLONE_JSON, object_key_val) {
  auto json_str = R"( { "A": 1, "B": 2, "C": 3 } )"_padded;
  constexpr std::array<std::pair<std::string_view, uint64_t>, 3> expectations =
      {{{"A"sv, 1ul}, {"B"sv, 2ul}, {"C"sv, 3ul}}};

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_TRUE(json_val.has_value());

  auto exp_clone = json_val.value().clone();
  EXPECT_TRUE(exp_clone.has_value());

  auto exp_obj = exp_clone.value();
  EXPECT_TRUE(exp_obj.get_type() == JsonTypes::Object);

  int count = 0;
  auto exp_iter = expectations.begin();
  for (const auto& [key, val] : exp_obj.read_object().value()) {
    EXPECT_EQ(key, exp_iter->first);
    EXPECT_EQ(val.get_type(), JsonTypes::Number);
    EXPECT_EQ(val.read_u64(), exp_iter->second);

    ++exp_iter;
    ++count;
  }
  EXPECT_EQ(count, 3);
}

TEST(CLONE_JSON, object_with_incr_arrays_of_types) {
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

  auto exp_clone = json_val.value().clone();
  EXPECT_TRUE(exp_clone.has_value());

  auto exp_obj = exp_clone.value();
  EXPECT_TRUE(exp_obj.get_type() == JsonTypes::Object);

  int count = 0;
  auto exp_iter = expectations.begin();
  for (const auto& [key, val] : exp_obj.read_object().value()) {
    EXPECT_EQ(key, std::get<0>(*exp_iter));

    EXPECT_EQ(val.get_type(), JsonTypes::Array);
    auto expSubarr = val.read_array();
    EXPECT_TRUE(expSubarr.has_value());

    int arrLen = 0;
    for (auto item : expSubarr.value()) {
      EXPECT_EQ(item.get_type(), std::get<1>(*exp_iter));
      ++arrLen;
    }

    EXPECT_EQ(arrLen, std::get<2>(*exp_iter));

    ++exp_iter;
    ++count;
  }
  EXPECT_EQ(count, 5);
}

#endif // USE_SIMD
