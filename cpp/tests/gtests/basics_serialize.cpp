
#include "generated/basic_disc.hpp"
#include "generated/basic_enum.hpp"
#include "generated/basic_struct.hpp"
#include "generated/primitives.hpp"

#include "stream_serializer.hpp"

#include <functional>
#include <gtest/gtest.h>
#include <sstream>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

namespace {

  namespace JW = JsonTypedefCodeGen::Writer;

  using OpFunc = std::function<ExpType<void>(JW::Serializer&)>;

  ExpType<void> execute_raw(JW::StreamSerializer& str_ser, OpFunc f) {
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

  auto create_array_string_ser(std::ostream* os) {
    JW::StreamSerializerCreateInfo info;
    info.start_as_array = true;
    info.output_stream = os;
    return JW::StreamSerializer::create(info);
  }

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
