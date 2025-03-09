#ifdef USE_SIMD

#include "generated/basic_disc.hpp"
#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "simd.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpJsonValue = ExpType<Reader::JsonValue>;
  using ExpBasicEnum = ExpType<test::BasicEnum>;
  using ExpBasicStruct = ExpType<test::BasicStruct>;
  using ExpBasicDisc = ExpType<test::BasicDisc>;

  void exp_error(const JsonError& err, const JsonError& exp_err) {
    EXPECT_EQ(err.type, exp_err.type);
    EXPECT_EQ(err.message, exp_err.message);
  }

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
          return make_json_error(JsonErrorTypes::Invalid);
        }));

    return flatten_expected(exp_enum.transform(test::deserialize_BasicEnum));
  }

  ExpBasicStruct get_exp_basic_struct(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::deserialize_BasicStruct(val);
                                }));
  }

  ExpBasicDisc get_exp_basic_disc(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::deserialize_BasicDisc(val);
                                }));
  }

} // namespace

TEST(BASIC, enum_ok) {
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
}

TEST(BASIC, enum_err) {
  {
    auto exp_be = get_exp_basic_enum(R"( ["Gary"] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
    exp_error(exp_be.error(),
              JsonError(JsonErrorTypes::Invalid,
                        "Invalid value \"Gary\" for BasicEnum"sv));
  }
}

TEST(BASIC, struct_ok) {
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
}

TEST(BASIC, struct_err) {
  // missing mandatory field "baz"
  {
    auto exp_bs = get_exp_basic_struct(
        R"( {
        "bar": "Bar",
        "foo": true
      } )"_padded);

    EXPECT_FALSE(exp_bs.has_value());
    exp_error(exp_bs.error(),
              JsonError(JsonErrorTypes::String,
                        "Missing key \"baz\" for BasicStruct"sv));
  }
  // duplicate "bar" item
  {
    auto exp_bs = get_exp_basic_struct(
        R"( {
        "bar": "Bar",
        "baz": [],
        "foo": false,
        "bar": "Bad"
      } )"_padded);

    EXPECT_FALSE(exp_bs.has_value());
    exp_error(exp_bs.error(),
              JsonError(JsonErrorTypes::Invalid, "Duplicated key bar"sv));
  }
  // invalid value
  {
    auto exp_bs = get_exp_basic_struct(R"( { "boo": "Bar" } )"_padded);

    EXPECT_FALSE(exp_bs.has_value());
    exp_error(exp_bs.error(),
              JsonError(JsonErrorTypes::Invalid,
                        "Invalid key \"boo\" in BasicStruct"sv));
  }
}

TEST(BASIC, discriminator_ok) {
  using Types = test::BasicDisc::Types;
  {
    auto exp_bd = get_exp_basic_disc(
        R"( {
        "Type": "Boolean",
        "quuz": false
        } )"_padded);

    EXPECT_TRUE(exp_bd.has_value());

    auto pd = std::move(exp_bd.value());
    EXPECT_EQ(pd.type(), Types::Boolean);

    auto pdb = pd.get<Types::Boolean>();
    EXPECT_NE(pdb, nullptr);
    EXPECT_FALSE(pdb->quuz);

    EXPECT_EQ(pd.get<Types::String>(), nullptr);
  }
  // "Type" not specified first
  {
    auto exp_bd = get_exp_basic_disc(
        R"( {
        "quuz": true,
        "Type": "Boolean"
        } )"_padded);

    EXPECT_TRUE(exp_bd.has_value());

    auto pd = std::move(exp_bd.value());
    EXPECT_EQ(pd.type(), Types::Boolean);

    auto pdb = pd.get<Types::Boolean>();
    EXPECT_NE(pdb, nullptr);
    EXPECT_TRUE(pdb->quuz);

    EXPECT_EQ(pd.get<Types::String>(), nullptr);
  }
  {
    auto exp_bd = get_exp_basic_disc(
        R"( {
        "Type": "String",
        "baz": "Baz"
        } )"_padded);

    EXPECT_TRUE(exp_bd.has_value());

    auto pd = std::move(exp_bd.value());
    EXPECT_EQ(pd.type(), Types::String);
    EXPECT_EQ(pd.get<Types::Boolean>(), nullptr);

    auto pds = pd.get<Types::String>();
    EXPECT_NE(pds, nullptr);
    EXPECT_EQ(pds->baz, "Baz"sv);
  }
}

TEST(BASIC, discriminator_err) {
  using Types = test::BasicDisc::Types;
  // invalid "Type"
  {
    auto exp_bd = get_exp_basic_disc(R"( { "Type": "Bob" } )"_padded);

    EXPECT_FALSE(exp_bd.has_value());
    exp_error(exp_bd.error(), JsonError(JsonErrorTypes::Invalid,
                                        "Invalid key \"Bob\" in BasicDisc"sv));
  }
  // missing "baz"
  {
    auto exp_bd = get_exp_basic_disc(R"( { "Type": "String" } )"_padded);

    EXPECT_FALSE(exp_bd.has_value());
    exp_error(exp_bd.error(),
              JsonError(JsonErrorTypes::String,
                        "Missing key \"baz\" for BasicDiscString"sv));
  }
  // duplicated key
  {
    auto exp_bd = get_exp_basic_disc(
        R"( {
        "Type": "String",
        "baz": "Baz",
        "baz": "Boz"
        } )"_padded);

    EXPECT_FALSE(exp_bd.has_value());
    exp_error(exp_bd.error(),
              JsonError(JsonErrorTypes::String, "Duplicated key baz"sv));
  }
  // unknow key in "Boolean"
  {
    auto exp_bd =
        get_exp_basic_disc(R"( { "Type": "Boolean", "quu": false } )"_padded);

    EXPECT_FALSE(exp_bd.has_value());
    exp_error(exp_bd.error(),
              JsonError(JsonErrorTypes::Invalid,
                        "Invalid key \"quu\" in BasicDiscBoolean"sv));
  }
}

#endif // USE_SIMD
