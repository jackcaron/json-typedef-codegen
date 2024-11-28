#ifdef USE_SIMD

#include "simd_reader.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace JsonTypedefCodeGen;
using namespace simdjson;

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
      // std::cerr << "ERROR: " << err.message << "\n\n";
    }
    ++exp;
  }
  EXPECT_EQ(exp, 5);
}

// TODO: add deeper object/array test

#endif // USE_SIMD
