#include "value.hpp"

#include "../internal.hpp"
#include "array.hpp"

using namespace simdjson::ondemand;
using namespace std::string_view_literals;

static JsonTypes map_simd_types(json_type type) {
  switch (type) {
  case json_type::array:
    return JsonTypes::Array;
  case json_type::boolean:
    return JsonTypes::Bool;
  case json_type::null:
    return JsonTypes::Null;
  case json_type::number:
    return JsonTypes::Number;
  case json_type::object:
    return JsonTypes::Object;
  case json_type::string:
    return JsonTypes::String;
  }
  return JsonTypes::Invalid;
}

static JsonErrorTypes map_err_type(const simdjson::error_code err_type) {
  switch (err_type) {
  case simdjson::INCORRECT_TYPE:
    return JsonErrorTypes::WrongType;

  case simdjson::STRING_ERROR:
  case simdjson::UTF8_ERROR:
  case simdjson::UNESCAPED_CHARS:
  case simdjson::UNCLOSED_STRING:
    return JsonErrorTypes::String;

  case simdjson::NUMBER_ERROR:
  case simdjson::BIGINT_ERROR:
  case simdjson::NUMBER_OUT_OF_RANGE:
  case simdjson::INDEX_OUT_OF_BOUNDS:
    return JsonErrorTypes::Number;

  case simdjson::IO_ERROR:
    return JsonErrorTypes::InOut;

  case simdjson::CAPACITY:
  case simdjson::MEMALLOC:
  case simdjson::TAPE_ERROR:
  case simdjson::DEPTH_ERROR:
  case simdjson::T_ATOM_ERROR:
  case simdjson::F_ATOM_ERROR:
  case simdjson::N_ATOM_ERROR:
  case simdjson::UNINITIALIZED:
  case simdjson::EMPTY:
  case simdjson::UNSUPPORTED_ARCHITECTURE:
  case simdjson::NO_SUCH_FIELD:
  case simdjson::INVALID_JSON_POINTER:
  case simdjson::INVALID_URI_FRAGMENT:
  case simdjson::UNEXPECTED_ERROR:
  case simdjson::PARSER_IN_USE:
  case simdjson::OUT_OF_ORDER_ITERATION:
  case simdjson::INSUFFICIENT_PADDING:
  case simdjson::INCOMPLETE_ARRAY_OR_OBJECT:
  case simdjson::SCALAR_DOCUMENT_AS_VALUE:
  case simdjson::OUT_OF_BOUNDS:
  case simdjson::TRAILING_CONTENT:
    return JsonErrorTypes::Internal;

  default:
    break;
  }
  return JsonErrorTypes::Invalid;
}

UnexpJsonError makeJsonError(const simdjson::error_code err_type) {
  return makeJsonError(map_err_type(err_type),
                       std::string(simdjson::error_message(err_type)));
}

JsonTypes SimdValue::get_type() const { return map_simd_types(m_value.type()); }

ExpType<bool> SimdValue::is_null() const {
  return m_value.type() == json_type::null;
}

ExpType<bool> SimdValue::read_bool() const {
  return map_simd_data(m_value.get_bool());
}

ExpType<double> SimdValue::read_double() const {
  return map_simd_data(m_value.get_double());
}

ExpType<uint64_t> SimdValue::read_u64() const {
  return map_simd_data(m_value.get_uint64());
}

ExpType<int64_t> SimdValue::read_i64() const {
  return map_simd_data(m_value.get_int64());
}

ExpType<std::string> SimdValue::read_str() const {
  return transform_expected<std::string>(
      map_simd_data(m_value.get_string()),
      [](const std::string_view sv) { return std::string(sv); });
}

ExpType<JsonArray> SimdValue::read_array() const {
  return transform_expected<JsonArray>(
      map_simd_data(m_value.get_array()),
      [](simdjson::ondemand::array arr) { return SimdArray::create(arr); });
}

ExpType<JsonObject> SimdValue::read_object() const {
  if (m_value.type() == json_type::object) {
    // return m_value.get_object();
  }
  return makeJsonError(JsonErrorTypes::WrongType, "not an object"sv);

  // return transform_expected<JsonArray>(
  //     map_simd_data(m_value.get_object()),
  //     [](simdjson::ondemand::object obj) { return SimdObject::create(obj);
  //     });
}

JsonValue SimdValue::create(simdjson::ondemand::value& val) {
  return create_json(std::move(std::make_unique<SimdValue>(val)));
}
