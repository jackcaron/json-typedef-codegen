#pragma once

#ifdef USE_IN_NLOH

#include "json_reader.hpp"
#include "nlohmann/json.hpp"

namespace JsonTypedefCodeGen::Reader {

  ExpType<JsonValue> nlohmann_root_value(const nlohmann::json root);

}

#endif