#ifdef USE_SIMD

#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "simd_reader.hpp"

#include <array>
#include <gtest/gtest.h>

// TO KEEP IN THE END:
#include <iostream>
// *****************

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpJsonValue = ExpType<Reader::JsonValue>;
  using ExpBasicEnum = ExpType<test::BasicEnum>;
  using ExpBasicStruct = ExpType<test::BasicStruct>;

  ExpBasicEnum get_exp_basic_enum(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    const auto exp_arr =
        flatten_expected(Reader::simdjson_root_value(doc.get_value())
                             .transform([](const Reader::JsonValue& val) {
                               return val.read_array();
                             }));

    const auto exp_enum = flatten_expected(exp_arr.transform(
        [](const Reader::JsonArray& arr) -> ExpType<Reader::JsonValue> {
          for (auto item : arr) {
            return item;
          }
          return makeJsonError(JsonErrorTypes::Invalid);
        }));

    return flatten_expected(exp_enum.transform(test::fromJsonBasicEnum));
  }

  ExpBasicStruct get_exp_basic_struct(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::fromJsonBasicStruct(val);
                                }));
  }

} // namespace

TEST(GEN_BASIC, enum) {
  {
    auto exp_be = get_exp_basic_enum(R"( [] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
    EXPECT_EQ(exp_be.error().type, JsonErrorTypes::Invalid);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Bar"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_EQ(exp_be.value(), test::BasicEnum::Bar);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Baz"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_EQ(exp_be.value(), test::BasicEnum::Baz);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Foo"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_EQ(exp_be.value(), test::BasicEnum::Foo);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Gary"] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
    EXPECT_EQ(exp_be.error().type, JsonErrorTypes::Invalid);
    EXPECT_EQ(exp_be.error().message, "Invalid value \"Gary\" for BasicEnum"sv);
  }
}

TEST(GEN_BASIC, struct) {
  {
    auto exp_bs = get_exp_basic_struct(
        R"( {
        "bar": "Bar",
        "baz": [],
        "foo": false
      } )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_EQ(bs.bar, "Bar");
    EXPECT_TRUE(bs.baz.empty());
    EXPECT_FALSE(bs.foo);
  }
  {
    auto exp_bs = get_exp_basic_struct(
        R"( {
        "bar": "",
        "baz": [true,false],
        "foo": true
      } )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_EQ(bs.bar, "");
    EXPECT_FALSE(bs.baz.empty());
    EXPECT_TRUE(bs.baz[0]);
    EXPECT_FALSE(bs.baz[1]);
    EXPECT_TRUE(bs.foo);
  }
  // missing mandatory field "baz"
  {
    auto exp_bs = get_exp_basic_struct(
        R"( {
        "bar": "Bar",
        "foo": true
      } )"_padded);

    EXPECT_FALSE(exp_bs.has_value());
    EXPECT_EQ(exp_bs.error().type, JsonErrorTypes::String);
    EXPECT_EQ(exp_bs.error().message, "Missing key \"baz\" for BasicStruct");
  }
}

#endif // USE_SIMD
