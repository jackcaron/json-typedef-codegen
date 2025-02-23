#ifdef USE_SIMD

#include "generated/basic_enum.hpp"
#include "simd_reader.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpJsonValue = ExpType<Reader::JsonValue>;
  using ExpBasicEnum = ExpType<test::BasicEnum>;

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

} // namespace

TEST(GEN_BASIC, enum) {
  {
    auto exp_be = get_exp_basic_enum(R"( [] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Bar"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_TRUE(exp_be.value() == test::BasicEnum::Bar);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Baz"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_TRUE(exp_be.value() == test::BasicEnum::Baz);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Foo"] )"_padded);
    EXPECT_TRUE(exp_be.has_value());
    EXPECT_TRUE(exp_be.value() == test::BasicEnum::Foo);
  }
  {
    auto exp_be = get_exp_basic_enum(R"( ["Gary"] )"_padded);
    EXPECT_FALSE(exp_be.has_value());
  }
}

#endif // USE_SIMD
