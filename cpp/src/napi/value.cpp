#include "value.hpp"

#include "../internal.hpp"
#include "array.hpp"
#include "object.hpp"

using namespace std::string_view_literals;

static constexpr JsonTypes map_napi_type(const napi_valuetype type) {
  switch (type) {
  case napi_undefined:
  case napi_null:
    return JsonTypes::Null;
  case napi_boolean:
    return JsonTypes::Bool;
  case napi_number:
  case napi_bigint:
    return JsonTypes::Number;
  case napi_string:
  case napi_symbol:
    return JsonTypes::String;
  case napi_object:
    return JsonTypes::Object;
  case napi_function:
  case napi_external:
  default:
    return JsonTypes::Invalid;
  }
}

static constexpr JsonErrorTypes map_err_type(const napi_status err_type) {
  switch (err_type) {
  case napi_object_expected:
  case napi_boolean_expected:
  case napi_array_expected:
  case napi_arraybuffer_expected:
  case napi_date_expected:
    return JsonErrorTypes::WrongType;

  case napi_number_expected:
  case napi_bigint_expected:
    return JsonErrorTypes::Number;

  case napi_string_expected:
  case napi_name_expected:
    return JsonErrorTypes::String;

  case napi_invalid_arg:
  case napi_function_expected:
  case napi_generic_failure:
  case napi_pending_exception:
  case napi_cancelled:
  case napi_escape_called_twice:
  case napi_handle_scope_mismatch:
  case napi_callback_scope_mismatch:
  case napi_queue_full:
  case napi_closing:
  case napi_detachable_arraybuffer_expected:
  case napi_would_deadlock:
  case napi_no_external_buffers_allowed:
  case napi_cannot_run_js:
    return JsonErrorTypes::Internal;

  default:
    return JsonErrorTypes::Invalid;
  }
}

// -------------------------------------------
JsonTypes NapiValue::get_type() const {
  if (m_value.IsEmpty()) {
    return JsonTypes::Invalid;
  } else if (m_value.IsArray()) {
    return JsonTypes::Array;
  } else {
    return map_napi_type(m_value.Type());
  }
}

ExpType<bool> NapiValue::is_null() const {
  return m_value.IsNull() || m_value.IsUndefined();
}

ExpType<bool> NapiValue::read_bool() const {
  if (m_value.IsBoolean()) {
    return m_value.ToBoolean();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not a boolean"sv);
}

ExpType<double> NapiValue::read_double() const {
  if (m_value.IsBigInt()) {
    bool lossless = false;
    return double(m_value.As<Napi::BigInt>().Int64Value(&lossless));
  } else if (m_value.IsNumber()) {
    return m_value.ToNumber().DoubleValue();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<uint64_t> NapiValue::read_u64() const {
  if (m_value.IsBigInt()) {
    bool lossless = false;
    return m_value.As<Napi::BigInt>().Uint64Value(&lossless);
  } else if (m_value.IsNumber()) {
    return m_value.ToNumber().Uint32Value();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<int64_t> NapiValue::read_i64() const {
  if (m_value.IsBigInt()) {
    bool lossless = false;
    return m_value.As<Napi::BigInt>().Int64Value(&lossless);
  } else if (m_value.IsNumber()) {
    return m_value.ToNumber().Int64Value();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<std::string> NapiValue::read_str() const {
  if (m_value.IsString() || m_value.IsSymbol()) {
    return m_value.ToString();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not a string"sv);
}

ExpType<JsonArray> NapiValue::read_array() const {
  if (m_value.IsArray()) {
    return NapiArray::create(m_value.As<Napi::Array>());
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not an array"sv);
}

ExpType<JsonObject> NapiValue::read_object() const {
  if (!m_value.IsObject()) {
    return makeJsonError(JsonErrorTypes::WrongType, "not an object"sv);
  } else if (m_value.IsArray()) {
    return makeJsonError(JsonErrorTypes::WrongType, "the object is an array"sv);
  } else {
    return NapiObject::create(m_value.As<Napi::Object>());
  }
}

JsonValue NapiValue::create(const Napi::Value val) {
  return create_json(std::move(std::make_unique<NapiValue>(val)));
}

// -------------------------------------------
UnexpJsonError makeJsonError(const napi_status err_type) {
  return makeJsonError(map_err_type(err_type));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Reader {

  DLL_PUBLIC ExpType<JsonValue> napi_root_value(const Napi::Value root) {
    if (root.IsArray() || root.IsObject()) {
      return NapiValue::create(root);
    }
    return makeJsonError(
        JsonErrorTypes::Invalid,
        "Expect root Napi::Value to be an object or an array"sv);
  }

} // namespace JsonTypedefCodeGen::Reader
