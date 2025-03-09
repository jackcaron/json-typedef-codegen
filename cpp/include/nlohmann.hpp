#pragma once

#if defined(USE_IN_NLOH) || defined(USE_OUT_NLOH)
#include "nlohmann/json.hpp"
#endif

#ifdef USE_IN_NLOH

#include "json_reader.hpp"

namespace JsonTypedefCodeGen::Reader {

  ExpType<JsonValue> nlohmann_root_value(const nlohmann::json root);

}

#endif

#ifdef USE_OUT_NLOH

namespace JsonTypedefCodeGen::Writer {

  //

} // namespace JsonTypedefCodeGen::Writer

#endif
