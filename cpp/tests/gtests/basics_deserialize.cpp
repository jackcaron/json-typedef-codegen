#ifdef USE_SIMD

#include "generated/basic_disc.hpp"
#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "generated/primitives.hpp"
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
  using ExpPrims = ExpType<test::Primitives>;

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

  ExpPrims get_exp_prims(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::deserialize_Primitives(val);
                                }));
  }

} // namespace

TEST(BASIC_DES, enum_ok) {
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

TEST(BASIC_DES, enum_err) {
  {
    auto exp_be = get_exp_basic_enum(R"( ["Gary"] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
    exp_error(exp_be.error(),
              JsonError(JsonErrorTypes::Invalid,
                        "Invalid value \"Gary\" for BasicEnum"sv));
  }
}

TEST(BASIC_DES, struct_ok) {
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

TEST(BASIC_DES, struct_err) {
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
              JsonError(JsonErrorTypes::String, "Duplicated key \"bar\""sv));
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

TEST(BASIC_DES, discriminator_ok) {
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

TEST(BASIC_DES, discriminator_err) {
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

TEST(BASIC_DES, primitives_ok) {
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 2,
          "i16": -3,
          "u32": 34,
          "f32": 3.34
        } )"_padded);

    EXPECT_TRUE(exp_prim.has_value());

    auto prim = std::move(exp_prim.value());
    EXPECT_EQ(prim.u8, 2);
    EXPECT_EQ(prim.i16, -3);
    EXPECT_EQ(prim.u32, 34);
    EXPECT_EQ(prim.f32, 3.34f);
  }
}

TEST(BASIC_DES, primitives_limits) {
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 300,
          "i16": -3,
          "u32": 34,
          "f32": 3.34
        } )"_padded);

    EXPECT_FALSE(exp_prim.has_value());
    exp_error(exp_prim.error(),
              JsonError(JsonErrorTypes::Number,
                        "Unsigned value 300 is greater than 255"sv));
  }
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 3,
          "i16": -60000,
          "u32": 34,
          "f32": 3.34
        } )"_padded);

    EXPECT_FALSE(exp_prim.has_value());
    exp_error(exp_prim.error(),
              JsonError(JsonErrorTypes::Number,
                        "Signed value -60000 outside limits -32768:32767"sv));
  }
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 3,
          "i16": -3,
          "u32": -34,
          "f32": 3.34
        } )"_padded);

    EXPECT_FALSE(exp_prim.has_value());
    exp_error(
        exp_prim.error(),
        JsonError(
            JsonErrorTypes::WrongType,
            "INCORRECT_TYPE: The JSON element does not have the requested type."sv));
  }
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 3,
          "i16": -3,
          "u32": 34,
          "f32": 300e50
        } )"_padded);

    EXPECT_FALSE(exp_prim.has_value());
    exp_error(
        exp_prim.error(),
        JsonError(
            JsonErrorTypes::Number,
            "Double value 3e+52 outside of float limits -3.4028234663852886e+38:3.4028234663852886e+38"sv));
  }
  {
    auto exp_prim = get_exp_prims(
        R"( {
          "u8": 3,
          "i16": -3,
          "u32": 34,
          "f32": 3e-40
        } )"_padded);

    EXPECT_FALSE(exp_prim.has_value());
    exp_error(
        exp_prim.error(),
        JsonError(
            JsonErrorTypes::Number,
            "Double value 3e-40 outside of float zeroes limits -1.1754943508222875e-38:1.1754943508222875e-38"sv));
  }
}

#endif // USE_SIMD
