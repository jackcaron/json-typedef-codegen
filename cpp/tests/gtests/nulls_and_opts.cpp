#ifdef USE_SIMD

#include "generated/nullable_enum.hpp"
#include "generated/nullable_struct.hpp"
#include "generated/optional_props.hpp"
#include "simd.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpJsonValue = ExpType<Reader::JsonValue>;
  using ExpNullableEnum = ExpType<test::NullableEnum>;
  using ExpNullableStruct = ExpType<test::NullableStruct>;
  using ExpOptProps = ExpType<test::OptionalProps>;

  ExpNullableEnum get_exp_null_enum(const padded_str& json_str) {
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

    return flatten_expected(
        exp_enum.transform(test::fromJsonNullableNullableEnum));
  }

  ExpNullableStruct get_exp_null_struct(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    const auto exp_arr =
        flatten_expected(Reader::simdjson_root_value(doc.get_value())
                             .transform([](const Reader::JsonValue& val) {
                               return val.read_array();
                             }));

    const auto exp_struct = flatten_expected(exp_arr.transform(
        [](const Reader::JsonArray& arr) -> ExpType<Reader::JsonValue> {
          for (auto item : arr) {
            return item;
          }
          return makeJsonError(JsonErrorTypes::Invalid);
        }));

    return flatten_expected(
        exp_struct.transform(test::fromJsonNullableNullableStruct));
  }

  ExpOptProps get_exp_opt_props(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::fromJsonOptionalProps(val);
                                }));
  }

} // namespace

TEST(NULL_AND_OPT, null_enum) {
  {
    auto exp_ne = get_exp_null_enum(R"( ["Bar"] )"_padded);
    EXPECT_TRUE(exp_ne.has_value());

    auto ne = std::move(exp_ne.value());
    EXPECT_NE(ne.get(), nullptr);
    EXPECT_EQ(*ne, test::NullableEnum0::Bar);
  }
  {
    auto exp_ne = get_exp_null_enum(R"( [null] )"_padded);
    EXPECT_TRUE(exp_ne.has_value());

    auto ne = std::move(exp_ne.value());
    EXPECT_EQ(ne.get(), nullptr);
  }
}

TEST(NULL_AND_OPT, null_struct) {
  {
    auto exp_bs = get_exp_null_struct(
        R"( [{
        "bar": "Bar",
        "baz": [],
        "foo": false
      }] )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_NE(bs.get(), nullptr);
    EXPECT_EQ(bs->bar, "Bar");
    EXPECT_TRUE(bs->baz.empty());
    EXPECT_FALSE(bs->foo);
  }
  {
    auto exp_bs = get_exp_null_struct(
        R"( [{
        "bar": "",
        "baz": [true,false],
        "foo": true
      }] )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_NE(bs.get(), nullptr);
    EXPECT_EQ(bs->bar, "");
    EXPECT_FALSE(bs->baz.empty());
    EXPECT_TRUE(bs->baz[0]);
    EXPECT_FALSE(bs->baz[1]);
    EXPECT_TRUE(bs->foo);
  }
  {
    auto exp_ne = get_exp_null_struct(R"( [null] )"_padded);
    EXPECT_TRUE(exp_ne.has_value());

    auto ne = std::move(exp_ne.value());
    EXPECT_EQ(ne.get(), nullptr);
  }
}

TEST(NULL_AND_OPT, opt_props_struct) {
  {
    auto exp_bs = get_exp_opt_props(
        R"( {
        "TrueFalse": true,
        "foo": "Bob"
      } )"_padded);

    EXPECT_FALSE(exp_bs.has_value());
    EXPECT_EQ(exp_bs.error().type, JsonErrorTypes::String);
  }
  {
    auto exp_bs = get_exp_opt_props(
        R"( {
        "Message": "Bar",
        "TrueFalse": true
      } )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_EQ(bs.message, "Bar");
    EXPECT_TRUE(bs.true_false);
    EXPECT_EQ(bs.baz.get(), nullptr);
    EXPECT_EQ(bs.foo.get(), nullptr);
  }
  {
    auto exp_bs = get_exp_opt_props(
        R"( {
        "Message": "Bar1",
        "TrueFalse": false,
        "foo": "Bob"
      } )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_EQ(bs.message, "Bar1");
    EXPECT_FALSE(bs.true_false);
    EXPECT_EQ(bs.baz.get(), nullptr);
    EXPECT_NE(bs.foo.get(), nullptr);
    EXPECT_EQ(*(bs.foo), "Bob");
  }
  {
    auto exp_bs = get_exp_opt_props(
        R"( {
        "Message": "Bar2",
        "TrueFalse": false,
        "baz": true
      } )"_padded);

    EXPECT_TRUE(exp_bs.has_value());

    auto bs = std::move(exp_bs.value());
    EXPECT_EQ(bs.message, "Bar2");
    EXPECT_FALSE(bs.true_false);
    EXPECT_NE(bs.baz.get(), nullptr);
    EXPECT_TRUE(*(bs.baz));
    EXPECT_EQ(bs.foo.get(), nullptr);
  }
}

#endif
