
#include "generated/basic_disc.hpp"
#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "generated/primitives.hpp"

#include "common_serialization.hpp"
#include "stream_serializer.hpp"

#include <gtest/gtest.h>
#include <sstream>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

namespace {

  namespace JW = JsonTypedefCodeGen::Writer;

  ExpType<void> execute_raw(JW::StreamSerializer& str_ser,
                            test::common::OpFunc f) {
    if (auto exp_ser = JW::to_stream_serializer(str_ser); exp_ser.has_value()) {
      if (auto exp_ok = f(exp_ser.value()); exp_ok.has_value()) {
        return str_ser.close();
      } else {
        return UnexpJsonError(exp_ok.error());
      }
    } else {
      return UnexpJsonError(exp_ser.error());
    }
  }

  auto create_array_string_ser(std::ostream* os) {
    JW::StreamSerializerCreateInfo info;
    info.start_as_array = true;
    info.open_root_item = false;
    info.output_stream = os;
    return JW::StreamSerializer::create(info);
  }

  auto create_object_string_ser(std::ostream* os) {
    JW::StreamSerializerCreateInfo info;
    info.start_as_array = false;
    info.open_root_item = false;
    info.output_stream = os;
    return JW::StreamSerializer::create(info);
  }

} // namespace

namespace test::common {

  ExpType<std::string> execute_as_array(OpFunc f) {
    std::stringstream ss;
    if (auto exp_str_ser = create_array_string_ser(&ss);
        !exp_str_ser.has_value()) {
      return UnexpJsonError(exp_str_ser.error());
    } else if (auto exp_ok = execute_raw(exp_str_ser.value(), f);
               exp_ok.has_value()) {
      return ss.str();
    } else {
      return UnexpJsonError(exp_ok.error());
    }
  }

  ExpType<std::string> execute_as_object(OpFunc f) {
    std::stringstream ss;
    if (auto exp_str_ser = create_object_string_ser(&ss);
        !exp_str_ser.has_value()) {
      return UnexpJsonError(exp_str_ser.error());
    } else if (auto exp_ok = execute_raw(exp_str_ser.value(), f);
               exp_ok.has_value()) {
      return ss.str();
    } else {
      return UnexpJsonError(exp_ok.error());
    }
  }

  void serialize_and_expected_json(OpFunc f,
                                   const std::string_view expected_json) {
    auto exp_str = execute_as_array(f);
    EXPECT_TRUE(exp_str.has_value());
    EXPECT_EQ(exp_str.value(), expected_json);
  }

} // namespace test::common

using namespace test::common;

TEST(BASIC_SER, invalid_stream_serializer) {
  JW::StreamSerializerCreateInfo info;
  auto exp_err = JW::StreamSerializer::create(info);

  EXPECT_FALSE(exp_err.has_value());
}

TEST(BASIC_SER, cannot_write_key_in_array) {
  auto exp_err = execute_as_array([](auto& serializer) {
    return serializer.write_key("Bob"sv);
  });
  EXPECT_FALSE(exp_err.has_value());
}

TEST(BASIC_SER, enum_ok) {
  serialize_and_expected_json(
      [](auto& serializer) {
        return test::serialize_BasicEnum(serializer, test::BasicEnum::Bar);
      },
      "\"Bar\""sv);
}

TEST(BASIC_SER, struct_ok) {
  serialize_and_expected_json(
      [](auto& serializer) {
        test::BasicStruct basic{
            .bar = "Bob", .baz = {true, false}, .foo = true};
        return test::serialize_BasicStruct(serializer, basic);
      },
      "{\"bar\": \"Bob\",\"baz\": [true,false],\"foo\": true}"sv);
}

TEST(BASIC_SER, disc_ok) {
  serialize_and_expected_json(
      [](auto& serializer) {
        const test::BasicDiscBoolean bdbool{.quuz = false};
        test::BasicDisc disc(bdbool);
        return test::serialize_BasicDisc(serializer, disc);
      },
      "{\"Type\": \"Boolean\",\"quuz\": false}"sv);

  serialize_and_expected_json(
      [](auto& serializer) {
        const test::BasicDiscString bdstr{.baz = "Bob"};
        test::BasicDisc disc(bdstr);
        return test::serialize_BasicDisc(serializer, disc);
      },
      "{\"Type\": \"String\",\"baz\": \"Bob\"}"sv);
}
