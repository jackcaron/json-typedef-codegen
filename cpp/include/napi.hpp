#pragma once

#if defined(USE_IN_NAPI) || defined(USE_OUT_NAPI)
#include <napi.h>
#endif

#ifdef USE_IN_NAPI

#include "json_reader.hpp"

namespace JsonTypedefCodeGen::Reader {

  ExpType<JsonValue> napi_root_value(const Napi::Value root);

} // namespace JsonTypedefCodeGen::Reader

#endif

#ifdef USE_OUT_NAPI

namespace JsonTypedefCodeGen::Writer {

  //

} // namespace JsonTypedefCodeGen::Writer

#endif
