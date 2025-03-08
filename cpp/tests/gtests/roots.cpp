#ifdef USE_SIMD

#include "generated/root_empty.hpp"
#include "generated/root_string.hpp"
#include "simd_reader.hpp"

#include <array>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpJsonValue = ExpType<Reader::JsonValue>;
  using ExpEmpty = ExpType<test::RootEmpty>;
  using ExpString = ExpType<test::RootString>;

  ExpEmpty get_exp_empty(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const Reader::JsonValue& val) {
                                  return test::fromJsonRootEmpty(val);
                                }));
  }

  ExpString get_exp_string(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);

    const auto exp_arr =
        flatten_expected(Reader::simdjson_root_value(doc.get_value())
                             .transform([](const Reader::JsonValue& val) {
                               return val.read_array();
                             }));

    const auto exp_str = flatten_expected(exp_arr.transform(
        [](const Reader::JsonArray& arr) -> ExpType<Reader::JsonValue> {
          for (auto item : arr) {
            return item;
          }
          return makeJsonError(JsonErrorTypes::Invalid);
        }));

    return flatten_expected(exp_str.transform(test::fromJsonRootString));
  }

} // namespace

TEST(ROOT, empty) {
  {
    auto exp_empty = get_exp_empty(R"( [] )"_padded);
    EXPECT_TRUE(exp_empty.has_value());

    auto empty = std::move(exp_empty.value());
    EXPECT_EQ(empty.get_type(), JsonTypes::Array);

    auto exp_array = empty.read_array();
    EXPECT_TRUE(exp_array.has_value());

    auto intern_array = exp_array.value().internal();
    EXPECT_TRUE(intern_array.empty());
  }
  {
    auto exp_empty = get_exp_empty(R"( [1,2] )"_padded);
    EXPECT_TRUE(exp_empty.has_value());

    auto empty = std::move(exp_empty.value());
    EXPECT_EQ(empty.get_type(), JsonTypes::Array);

    auto exp_array = empty.read_array();
    EXPECT_TRUE(exp_array.has_value());

    auto intern_array = exp_array.value().internal();
    EXPECT_FALSE(intern_array.empty());
    EXPECT_EQ(intern_array.size(), 2ul);
  }
  {
    auto exp_empty = get_exp_empty(R"( {} )"_padded);
    EXPECT_TRUE(exp_empty.has_value());

    auto empty = std::move(exp_empty.value());
    EXPECT_EQ(empty.get_type(), JsonTypes::Object);

    auto exp_obj = empty.read_object();
    EXPECT_TRUE(exp_obj.has_value());

    auto intern_obj = exp_obj.value().internal();
    EXPECT_TRUE(intern_obj.empty());
  }
  {
    auto exp_empty = get_exp_empty(R"( {"a":1} )"_padded);
    EXPECT_TRUE(exp_empty.has_value());

    auto empty = std::move(exp_empty.value());
    EXPECT_EQ(empty.get_type(), JsonTypes::Object);

    auto exp_obj = empty.read_object();
    EXPECT_TRUE(exp_obj.has_value());

    auto intern_obj = exp_obj.value().internal();
    EXPECT_FALSE(intern_obj.empty());
    EXPECT_EQ(intern_obj.size(), 1ul);
    EXPECT_TRUE(intern_obj.contains("a"));
  }
}

TEST(ROOT, string) {
  {
    auto exp_str = get_exp_string(R"( [] )"_padded);
    EXPECT_FALSE(exp_str.has_value());
    EXPECT_EQ(exp_str.error().type, JsonErrorTypes::Invalid);
  }
  {
    auto exp_str = get_exp_string(R"( [1] )"_padded);
    EXPECT_FALSE(exp_str.has_value());
    EXPECT_EQ(exp_str.error().type, JsonErrorTypes::WrongType);
  }
  {
    auto exp_str = get_exp_string(R"( ["Bob"] )"_padded);
    EXPECT_TRUE(exp_str.has_value());

    auto str = std::move(exp_str.value());
    EXPECT_EQ(str.compare("Bob"), 0);
  }
}

#endif