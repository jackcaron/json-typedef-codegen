#pragma once

#ifdef USE_SIMD

#include "json_reader.hpp"
#include "simd_json/simdjson.h"

namespace JsonTypedefCodeGen::Reader {

  ExpType<JsonValue> simdjson_root_value(
      simdjson::simdjson_result<simdjson::ondemand::value> root);

} // namespace JsonTypedefCodeGen::Reader

#endif
