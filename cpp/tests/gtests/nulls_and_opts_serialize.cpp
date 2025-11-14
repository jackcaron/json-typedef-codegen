
#include "generated/nullable_enum.hpp"
#include "generated/nullable_struct.hpp"
#include "generated/optional_props.hpp"

#include "common_serialization.hpp"
#include "stream_serializer.hpp"

#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;
using namespace test::common;

TEST(NULL_AND_OPT_SER, null_enum) {
  serialize_and_expected_json(
      [](auto& serializer) {
        return test::serialize_NullableEnum(serializer, nullptr);
      },
      "null"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        auto ve =
            std::make_unique<test::NullableEnum0>(test::NullableEnum0::Bar);
        return test::serialize_NullableEnum(serializer, ve);
      },
      "\"Bar\""sv);
}

TEST(NULL_AND_OPT_SER, null_struct) {
  serialize_and_expected_json(
      [](auto& serializer) {
        return test::serialize_NullableStruct(serializer, nullptr);
      },
      "null"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        auto ve = std::make_unique<test::NullableStruct0>(test::NullableStruct0{
            .bar = "Bob", .baz = {false, true}, .foo = true});
        return test::serialize_NullableStruct(serializer, ve);
      },
      "{\"bar\":\"Bob\",\"baz\":[false,true],\"foo\":true}"sv);
}

TEST(NULL_AND_OPT_SER, opt_props_struct) {
  serialize_and_expected_json(
      [](auto& serializer) {
        test::OptionalProps props;
        props.true_false = true;
        return test::serialize_OptionalProps(serializer, props);
      },
      "{\"Message\":\"\",\"TrueFalse\":true}"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        test::OptionalProps props;
        props.true_false = false;
        props.baz = std::make_unique<bool>(false);
        return test::serialize_OptionalProps(serializer, props);
      },
      "{\"Message\":\"\",\"TrueFalse\":false,\"baz\":false}"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        test::OptionalProps props;
        props.true_false = false;
        props.foo = std::make_unique<std::string>("bob"sv);
        return test::serialize_OptionalProps(serializer, props);
      },
      "{\"Message\":\"\",\"TrueFalse\":false,\"foo\":\"bob\"}"sv);
}