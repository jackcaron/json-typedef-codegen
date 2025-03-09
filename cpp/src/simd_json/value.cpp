#include "value.hpp"

#include "../internal.hpp"
#include "array.hpp"
#include "object.hpp"

using namespace simdjson::ondemand;

static constexpr JsonTypes map_simd_types(const json_type type) {
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
  default:
    return JsonTypes::Invalid;
  }
}

static constexpr JsonErrorTypes
map_err_type(const simdjson::error_code err_type) {
  switch (err_type) {
  case simdjson::INCORRECT_TYPE:
  case simdjson::SCALAR_DOCUMENT_AS_VALUE:
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
  case simdjson::OUT_OF_BOUNDS:
  case simdjson::TRAILING_CONTENT:
    return JsonErrorTypes::Internal;

  case simdjson::INCOMPLETE_ARRAY_OR_OBJECT:
  default:
    return JsonErrorTypes::Invalid;
  }
}

// -------------------------------------------
JsonTypes SimdValue::get_type() const { return map_simd_types(m_value.type()); }

ExpType<bool> SimdValue::is_null() const {
  auto tp = map_simd_data(m_value.type());
  if (tp.has_value()) {
    switch (tp.value()) {
    case json_type::null:
      return true;
    default:
      break;
    };
  }
  return false;
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
  return map_simd_data(m_value.get_string())
      .transform([](const std::string_view& sv) {
        return std::string(sv);
      });
}

ExpType<JsonArray> SimdValue::read_array() const {
  return map_simd_data(m_value.get_array()).transform(SimdArray::create);
}

ExpType<JsonObject> SimdValue::read_object() const {
  return map_simd_data(m_value.get_object()).transform(SimdObject::create);
}

NumberType SimdValue::get_number_type() const {
  if (m_value.type() == json_type::number) {
    if (m_value.is_integer()) {
      return m_value.is_negative() ? NumberType::I64 : NumberType::U64;
    }
    return NumberType::Double;
  }
  return NumberType::NaN;
}

JsonValue SimdValue::create(const simdjson::ondemand::value val) {
  return create_json(std::move(std::make_unique<SimdValue>(val)));
}

// -------------------------------------------
UnexpJsonError make_json_error(const simdjson::error_code err_type) {
  return make_json_error(map_err_type(err_type),
                         std::string(simdjson::error_message(err_type)));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Reader {

  using namespace simdjson;

  DLL_PUBLIC ExpType<JsonValue>
  simdjson_root_value(const simdjson::simdjson_result<ondemand::value> root) {
    if (const auto err_code = root.error(); err_code == simdjson::SUCCESS) {
      return SimdValue::create(root.value_unsafe());
    } else {
      return ::make_json_error(err_code);
    }
  }

} // namespace JsonTypedefCodeGen::Reader
