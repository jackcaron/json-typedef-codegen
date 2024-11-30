#ifdef USE_IN_NAPI

#include "napi.hpp"
#include "utils.hpp"

#include <iostream>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

static bool bypass_array(Napi::Value& val) {
  if (!val.IsArray()) {
    return false;
  }
  auto tmp_arr = val.As<Napi::Array>();
  if (tmp_arr.Length() != 1) {
    return false;
  }
  val = tmp_arr.Get(0u);
  return true;
}

NAPI_TEST(NAPI_IN, ignore_empty) {
  auto script_val = env.RunScript("");

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_FALSE(json_val.has_value());
  NAPI_EXP_TRUE(json_val.error().type == JsonErrorTypes::Invalid);
}

NAPI_TEST(NAPI_IN, ignore_non_array_object) {
  {
    auto script_val = env.RunScript(R"( true )");

    NAPI_EXP_TRUE(script_val.IsBoolean());

    auto json_val = Reader::napi_root_value(script_val);
    NAPI_EXP_FALSE(json_val.has_value());
    NAPI_EXP_TRUE(json_val.error().type == JsonErrorTypes::Invalid);
  }
  {
    auto script_val = env.RunScript(R"( 123 )");

    NAPI_EXP_TRUE(script_val.IsNumber());

    auto json_val = Reader::napi_root_value(script_val);
    NAPI_EXP_FALSE(json_val.has_value());
    NAPI_EXP_TRUE(json_val.error().type == JsonErrorTypes::Invalid);
  }
}

NAPI_TEST(NAPI_IN, empty_array) {
  auto script_val = env.RunScript(R"( [] )");

  NAPI_EXP_TRUE(script_val.IsArray());

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  NAPI_EXP_TRUE(val.get_type() == JsonTypes::Array);

  auto arr = val.read_array();
  NAPI_EXP_TRUE(arr.has_value());
  bool has_item = false;
  for (auto item : arr.value()) {
    has_item = true;
  }
  NAPI_EXP_FALSE(has_item);
}

NAPI_TEST(NAPI_IN, empty_object) {
  // eval() doesn't like object defined at the root, so putting it in an array
  auto script_val = env.RunScript(R"( [{}] )");

  NAPI_EXP_TRUE(bypass_array(script_val));

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  NAPI_EXP_TRUE(val.get_type() == JsonTypes::Object);

  auto obj = val.read_object();
  NAPI_EXP_TRUE(obj.has_value());
  bool has_item = false;
  for (auto item : obj.value()) {
    has_item = true;
  }
  NAPI_EXP_FALSE(has_item);
}

NAPI_TEST(NAPI_IN, array_of_numbers) {
  auto script_val = env.RunScript(R"( [1,2,3,4] )");

  NAPI_EXP_TRUE(script_val.IsArray());

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  NAPI_EXP_TRUE(val.get_type() == JsonTypes::Array);

  auto arr = val.read_array();
  NAPI_EXP_TRUE(arr.has_value());
  int64_t exp = 1;
  for (auto item : arr.value()) {
    NAPI_EXP_TRUE(item.has_value());
    auto val = std::move(item.value());
    NAPI_EXP_TRUE(val.get_type() == JsonTypes::Number);
    NAPI_EXP_TRUE(val.read_i64() == exp);
    ++exp;
  }
  NAPI_EXP_TRUE(exp == 5);
}

NAPI_TEST(NAPI_IN, object_key_val) {
  // eval() doesn't like object defined at the root, so putting it in an array
  auto script_val = env.RunScript(R"( [{ "A": 1, "B": 2, "C": 3 }] )");
  constexpr std::array<std::pair<std::string_view, uint64_t>, 3> expectations =
      {{{"A"sv, 1ul}, {"B"sv, 2ul}, {"C"sv, 3ul}}};

  NAPI_EXP_TRUE(bypass_array(script_val));

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  NAPI_EXP_TRUE(val.get_type() == JsonTypes::Object);

  auto obj = val.read_object();
  NAPI_EXP_TRUE(obj.has_value());

  int count = 0;
  auto exp_iter = expectations.begin();
  for (auto kvpair : obj.value()) {
    NAPI_EXP_TRUE(kvpair.has_value());
    auto [key, val] = std::move(kvpair.value());

    NAPI_EXP_TRUE(key == exp_iter->first);
    NAPI_EXP_TRUE(val.get_type() == JsonTypes::Number);
    NAPI_EXP_TRUE(val.read_u64() == exp_iter->second);

    ++exp_iter;
    ++count;
  }
  NAPI_EXP_TRUE(count == 3);
}

NAPI_TEST(NAPI_IN, object_with_incr_arrays_of_types) {
  // eval() doesn't like object defined at the root, so putting it in an array
  auto script_val = env.RunScript(R"( [{
    "Alice": [1],
    "Bob": [false, true],
    "Chuck": [{}, {}, {}],
    "Dave": [[],[],[],[]],
    "Elle": ["a", "b", "c", "d", "e"]
  }] )");

  constexpr std::array<std::tuple<std::string_view, JsonTypes, int>, 5>
      expectations = {{
          {"Alice"sv, JsonTypes::Number, 1},
          {"Bob"sv, JsonTypes::Bool, 2},
          {"Chuck"sv, JsonTypes::Object, 3},
          {"Dave"sv, JsonTypes::Array, 4},
          {"Elle"sv, JsonTypes::String, 5},
      }};

  NAPI_EXP_TRUE(bypass_array(script_val));

  auto json_val = Reader::napi_root_value(script_val);
  NAPI_EXP_TRUE(json_val.has_value());

  auto val = std::move(json_val.value());
  NAPI_EXP_TRUE(val.get_type() == JsonTypes::Object);

  auto obj = val.read_object();
  NAPI_EXP_TRUE(obj.has_value());

  int count = 0;
  auto exp_iter = expectations.begin();
  for (auto kvpair : obj.value()) {
    NAPI_EXP_TRUE(kvpair.has_value());
    auto [key, val] = std::move(kvpair.value());

    NAPI_EXP_TRUE(key == std::get<0>(*exp_iter));

    NAPI_EXP_TRUE(val.get_type() == JsonTypes::Array);
    auto expSubarr = val.read_array();
    NAPI_EXP_TRUE(expSubarr.has_value());

    int arrLen = 0;
    for (auto item : expSubarr.value()) {
      NAPI_EXP_TRUE(item.has_value());
      NAPI_EXP_TRUE(item.value().get_type() == std::get<1>(*exp_iter));
      ++arrLen;
    }

    NAPI_EXP_TRUE(arrLen == std::get<2>(*exp_iter));

    ++exp_iter;
    ++count;
  }
  NAPI_EXP_TRUE(count == 5);
}

#endif
