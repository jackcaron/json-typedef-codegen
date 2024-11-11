#include "../internal.hpp"
#include "json_reader.hpp"
#include "value.hpp"

using namespace simdjson;

namespace JsonTypedefCodeGen::Reader {

  DLL_PUBLIC ExpType<JsonValue> simdjson_root_value(ondemand::value& root) {
    return SimdValue::create(root);
  }

} // namespace JsonTypedefCodeGen::Reader
