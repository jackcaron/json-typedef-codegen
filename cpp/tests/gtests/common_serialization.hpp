#pragma once

#include <functional>
#include <json_writer.hpp>

namespace test::common {

  using OpFunc = std::function<JsonTypedefCodeGen::ExpType<void>(
      JsonTypedefCodeGen::Writer::Serializer&)>;

  // implemented in basic_serialize.cpp
  JsonTypedefCodeGen::ExpType<std::string> execute_as_array(OpFunc f);
  JsonTypedefCodeGen::ExpType<std::string> execute_as_object(OpFunc f);

  void serialize_and_expected_json(OpFunc f,
                                   const std::string_view expected_json);

} // namespace test::common
