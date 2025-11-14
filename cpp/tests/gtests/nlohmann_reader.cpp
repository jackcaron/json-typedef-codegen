#include "nlohmann.hpp"

#ifdef USE_IN_NLOH

#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;
using namespace nlohmann::json_literals;

TEST(NLOH_READ, invalid) {
  bool got_error = false;
  try {
    auto data = R"( )"_json;
  } catch (...) {
    got_error = true;
  }
  EXPECT_TRUE(got_error);
}

TEST(NLOH_READ, non_array_object) {
  {
    auto json_val = Reader::nlohmann_root_value(R"( true )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_bool = val.read_bool();

    EXPECT_TRUE(exp_bool.has_value());
    EXPECT_TRUE(exp_bool.value());
  }
  {
    auto json_val = Reader::nlohmann_root_value(R"( false )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_bool = val.read_bool();

    EXPECT_TRUE(exp_bool.has_value());
    EXPECT_FALSE(exp_bool.value());
  }
  {
    auto json_val = Reader::nlohmann_root_value(R"( null )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_null = val.is_null();

    EXPECT_TRUE(exp_null.has_value());
    EXPECT_TRUE(exp_null.value());
  }
  {
    auto json_val = Reader::nlohmann_root_value(R"( 123 )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_number = val.read_i64();

    EXPECT_TRUE(exp_number.has_value());
    EXPECT_EQ(exp_number.value(), 123ll);
  }
  {
    auto json_val = Reader::nlohmann_root_value(R"( 123 )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_number = val.read_double();

    EXPECT_TRUE(exp_number.has_value());
    EXPECT_EQ(exp_number.value(), 123.0);
  }
  {
    auto json_val = Reader::nlohmann_root_value(R"( "Bob" )"_json);
    EXPECT_TRUE(json_val.has_value());

    auto val = std::move(json_val.value());
    auto exp_str = val.read_str();

    EXPECT_TRUE(exp_str.has_value());
    EXPECT_EQ(exp_str.value(), "Bob");
  }
}

TEST(NLOH_READ, empty_array) {
  auto json_val = Reader::nlohmann_root_value(R"( [] )"_json);
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

TEST(NLOH_READ, empty_object) {
  auto json_val = Reader::nlohmann_root_value(R"( {} )"_json);
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);

  auto obj = val.read_object();
  EXPECT_TRUE(obj.has_value());
  bool has_item = false;
  for (auto item : obj.value()) {
    has_item = true;
  }
  EXPECT_FALSE(has_item);
}

TEST(NLOH_READ, array_of_numbers) {
  auto json_val = Reader::nlohmann_root_value(R"( [1,2,3,4] )"_json);
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

TEST(NLOH_READ, object_with_incr_arrays_of_types) {
  constexpr std::array<std::tuple<std::string_view, JsonTypes, int>, 5>
      expectations = {{
          {"Alice"sv, JsonTypes::Number, 1},
          {"Bob"sv, JsonTypes::Bool, 2},
          {"Chuck"sv, JsonTypes::Object, 3},
          {"Dave"sv, JsonTypes::Array, 4},
          {"Elle"sv, JsonTypes::String, 5},
      }};

  auto json_val = Reader::nlohmann_root_value(R"( {
    "Alice": [1],
    "Bob": [false, true],
    "Chuck": [{}, {}, {}],
    "Dave": [[],[],[],[]],
    "Elle": ["a", "b", "c", "d", "e"]
  } )"_json);
  EXPECT_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  EXPECT_EQ(val.get_type(), JsonTypes::Object);

  auto obj = val.read_object();
  EXPECT_TRUE(obj.has_value());

  int count = 0;
  auto exp_iter = expectations.begin();
  for (auto kvpair : obj.value()) {
    EXPECT_TRUE(kvpair.has_value());
    auto [key, val] = std::move(kvpair.value());

    EXPECT_EQ(key, std::get<0>(*exp_iter));

    EXPECT_EQ(val.get_type(), JsonTypes::Array);
    auto expSubarr = val.read_array();
    EXPECT_TRUE(expSubarr.has_value());

    int arrLen = 0;
    for (auto item : expSubarr.value()) {
      EXPECT_TRUE(item.has_value());
      EXPECT_EQ(item.value().get_type(), std::get<1>(*exp_iter));
      ++arrLen;
    }

    EXPECT_EQ(arrLen, std::get<2>(*exp_iter));

    ++exp_iter;
    ++count;
  }
  EXPECT_EQ(count, 5);
}

#endif
