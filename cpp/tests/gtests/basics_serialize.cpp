
#include "generated/basic_disc.hpp"
#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "generated/primitives.hpp"

#include "string_serializer.hpp"

#include <functional>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

namespace {

  namespace JW = JsonTypedefCodeGen::Writer;

  using OpFunc = std::function<ExpType<void>(JW::Serializer&)>;

  ExpType<void> execute_raw(JW::StringSerializer& str_ser, OpFunc f) {
    if (auto exp_ser = JW::to_string_serializer(str_ser); exp_ser.has_value()) {
      if (auto exp_ok = f(exp_ser.value()); exp_ok.has_value()) {
        return str_ser.close();
      } else {
        return UnexpJsonError(exp_ok.error());
      }
    } else {
      return UnexpJsonError(exp_ser.error());
    }
  }

  JW::StringSerializer create_array_string_ser() {
    JW::StringSerializerCreateInfo info;
    info.start_as_array = true;
    return JW::StringSerializer::create(info);
  }

  ExpType<std::string> execute_as_array(OpFunc f) {
    auto str_ser = create_array_string_ser();
    if (auto exp_ok = execute_raw(str_ser, f); exp_ok.has_value()) {
      return str_ser.to_string();
    } else {
      return UnexpJsonError(exp_ok.error());
    }
  }

  JW::StringSerializer create_object_string_ser() {
    JW::StringSerializerCreateInfo info;
    return JW::StringSerializer::create(info);
  }

  ExpType<std::string> execute_as_object(OpFunc f) {
    auto str_ser = create_array_string_ser();
    if (auto exp_ok = execute_raw(str_ser, f); exp_ok.has_value()) {
      return str_ser.to_string();
    } else {
      return UnexpJsonError(exp_ok.error());
    }
  }

} // namespace

TEST(BASIC_SER, enum_ok) {
  auto exp_str = execute_as_array([](auto& serializer) {
    return test::serialize_BasicEnum(serializer, test::BasicEnum::Bar);
  });

  EXPECT_TRUE(exp_str.has_value());
  EXPECT_EQ(exp_str.value(), "[\"Bar\"]"sv);
}

TEST(BASIC_SER, struct_ok) {
  auto exp_str = execute_as_array([](auto& serializer) {
    test::BasicStruct basic{.bar = "Bob", .baz = {true, false}, .foo = true};
    return test::serialize_BasicStruct(serializer, basic);
  });

  EXPECT_TRUE(exp_str.has_value());
  EXPECT_EQ(exp_str.value(),
            "[{\"bar\": \"Bob\",\"baz\": [true,false],\"foo\": true}]"sv);
}
