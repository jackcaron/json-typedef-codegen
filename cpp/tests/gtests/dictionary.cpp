#ifdef USE_SIMD

#include "generated/dictionary.hpp"
#include "common_serialization.hpp"
#include "simd.hpp"

#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace simdjson;
using namespace std::string_view_literals;

namespace {

  using padded_str = simdjson::padded_string;
  using ExpDictionary = ExpType<test::Dictionary>;

  ExpDictionary get_exp_dict(const padded_str& json_str) {
    ondemand::parser parser;
    auto doc = parser.iterate(json_str);
    return flatten_expected(Reader::simdjson_root_value(doc.get_value())
                                .transform([](const auto& val) {
                                  return test::deserialize_Dictionary(val);
                                }));
  }

} // namespace

using namespace test::common;

TEST(DictionaryDeserialize, error_missing_free) {
  auto exp_dict = get_exp_dict(R"( {} )"_padded);
  EXPECT_FALSE(exp_dict.has_value());
  EXPECT_EQ(exp_dict.error().type, JsonErrorTypes::String);
}

TEST(DictionaryDeserialize, error_free_null) {
  auto exp_dict = get_exp_dict(R"( { "free": null } )"_padded);
  EXPECT_FALSE(exp_dict.has_value());
  EXPECT_EQ(exp_dict.error().type, JsonErrorTypes::WrongType);
}

TEST(DictionaryDeserialize, empty) {
  auto exp_dict = get_exp_dict(R"( { "free": {} } )"_padded);
  EXPECT_TRUE(exp_dict.has_value());
  EXPECT_TRUE(exp_dict.value().free.empty());
}

TEST(DictionaryDeserialize, filled) {
  auto exp_dict = get_exp_dict(R"( { "free": { "bob": 12 } } )"_padded);
  EXPECT_TRUE(exp_dict.has_value());

  auto dict = std::move(exp_dict.value());
  EXPECT_FALSE(dict.free.empty());
  EXPECT_TRUE(dict.free.contains("bob"));

  auto bob = dict.free.at("bob");
  EXPECT_TRUE(bob.get_type() == JsonTypes::Number);
  EXPECT_EQ(bob.read_u64(), 12ul);
}

TEST(DictionarySerialize, empty) {
  serialize_and_expected_json(
      [](auto& serializer) {
        test::Dictionary dict = {.free = {}};
        return test::serialize_Dictionary(serializer, dict);
      },
      "{\"free\":{}}"sv);
}

TEST(DictionarySerialize, filled) {
  serialize_and_expected_json(
      [](auto& serializer) {
        test::Dictionary dict;
        dict.free.emplace("bob", Data::JsonValue(true));
        return test::serialize_Dictionary(serializer, dict);
      },
      "{\"free\":{\"bob\":true}}"sv);
}

#endif
