#ifdef USE_SIMD

#include "json_reader.hpp"
#include "simd_json/simdjson.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace JsonTypedefCodeGen;
using namespace simdjson;

static ExpType<Reader::JsonValue>
from_padded_string(simdjson::padded_string& json) {
  ondemand::parser parser;
  auto doc = parser.iterate(json);
  return Reader::simdjson_root_value(doc.get_value());
}

TEST(SIMD_JSON, ignore_empty) {
  auto json_str = R"( )"_padded;

  auto json_val = from_padded_string(json_str);
  EXPECT_EQ(json_val.has_value(), false);
  EXPECT_EQ(json_val.error().type, JsonErrorTypes::Internal);
}

TEST(SIMD_JSON, ignore_non_array_object) {
  {
    auto json_str = R"( true )"_padded;
    auto json_val = from_padded_string(json_str);
    EXPECT_EQ(json_val.has_value(), false);
    EXPECT_EQ(json_val.error().type, JsonErrorTypes::WrongType);
  }
  {
    auto json_str = R"( 123 )"_padded;

    auto json_val = from_padded_string(json_str);
    EXPECT_EQ(json_val.has_value(), false);
    EXPECT_EQ(json_val.error().type, JsonErrorTypes::WrongType);
  }
}

TEST(SIMD_JSON, empty_array) {
  auto json_str = R"( [] )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_EQ(json_val.has_value(), true);

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Array);
}

TEST(SIMD_JSON, empty_object) {
  auto json_str = R"( {} )"_padded;

  ondemand::parser parser;
  auto doc = parser.iterate(json_str);
  auto json_val = Reader::simdjson_root_value(doc.get_value());
  EXPECT_EQ(json_val.has_value(), true);

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);
}

//

//

#endif
