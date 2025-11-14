
#include "common_serialization.hpp"
#include "stream_serializer.hpp"

#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;
using namespace test::common;

TEST(JS_DATA_SER, array) {
  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonArray array;
        return serializer.write(array);
      },
      "[]"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonArray array;
        array.internal() = {Data::JsonValue()};
        return serializer.write(array);
      },
      "[null]"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonArray array;
        array.internal() = {Data::JsonValue(1.2), Data::JsonValue(false)};
        return serializer.write(array);
      },
      "[1.2,false]"sv);
}

TEST(JS_DATA_SER, object) {
  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonObject obj;
        return serializer.write(obj);
      },
      "{}"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonObject obj;
        obj.internal().emplace("bob"sv, Data::JsonValue());
        return serializer.write(obj);
      },
      "{\"bob\":null}"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        Data::JsonObject obj;
        auto& map = obj.internal();
        map.emplace("bob"sv, Data::JsonValue(1ul));
        map.emplace("alice"sv, Data::JsonValue("Power"sv));
        return serializer.write(obj);
      },
      "{\"alice\":\"Power\",\"bob\":1}"sv);
}