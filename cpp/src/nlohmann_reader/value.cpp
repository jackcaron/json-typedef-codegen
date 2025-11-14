#include "value.hpp"

#include "../internal.hpp"
#include "array.hpp"
#include "object.hpp"

using namespace std::string_view_literals;
namespace detail = nlohmann::detail;
using NType = detail::value_t;

static constexpr JsonTypes map_nloh_types(const NType type) {
  switch (type) {
  case NType::null:
    return JsonTypes::Null;

  case NType::object:
    return JsonTypes::Object;

  case NType::array:
    return JsonTypes::Array;

  case NType::string:
    return JsonTypes::String;

  case NType::boolean:
    return JsonTypes::Bool;

  case NType::number_integer:
  case NType::number_unsigned:
  case NType::number_float:
    return JsonTypes::Number;

  case NType::binary:
  case NType::discarded:
  default:
    return JsonTypes::Invalid;
  }
}

// -------------------------------------------
JsonTypes NlohValue::get_type() const { return map_nloh_types(m_value.type()); }

ExpType<bool> NlohValue::is_null() const {
  return m_value.type() == NType::null;
}

ExpType<bool> NlohValue::read_bool() const {
  if (m_value.is_boolean()) {
    return m_value.get<bool>();
  }
  return make_json_error(JsonErrorTypes::WrongType, "not a boolean"sv);
}

ExpType<double> NlohValue::read_double() const {
  if (m_value.is_number()) {
    return m_value.get<double>();
  }
  return make_json_error(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<uint64_t> NlohValue::read_u64() const {
  if (m_value.is_number()) {
    return m_value.get<uint64_t>();
  }
  return make_json_error(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<int64_t> NlohValue::read_i64() const {
  if (m_value.is_number()) {
    return m_value.get<int64_t>();
  }
  return make_json_error(JsonErrorTypes::WrongType, "not a number"sv);
}

ExpType<std::string> NlohValue::read_str() const {
  if (m_value.is_string()) {
    return m_value.get<std::string>();
  }
  return make_json_error(JsonErrorTypes::WrongType, "not a string"sv);
}

ExpType<JsonArray> NlohValue::read_array() const {
  if (m_value.is_array()) {
    return NlohArray::create(m_value.get<NlohVector>());
  }
  return make_json_error(JsonErrorTypes::WrongType, "not an array"sv);
}

ExpType<JsonObject> NlohValue::read_object() const {
  if (m_value.is_object()) {
    return NlohObject::create(m_value.get<NlohMap>());
  }
  return make_json_error(JsonErrorTypes::WrongType, "not an object"sv);
}

NumberType NlohValue::get_number_type() const {
  switch (m_value.type()) {
  case NType::number_float:
    return NumberType::Double;
  case NType::number_integer:
    return NumberType::I64;
  case NType::number_unsigned:
    return NumberType::U64;
  default:
    break;
  }
  return NumberType::NaN;
}

JsonValue NlohValue::create(const nlohmann::json value) {
  return create_json(std::move(std::make_unique<NlohValue>(value)));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Reader {

  DLL_PUBLIC ExpType<JsonValue> nlohmann_root_value(const nlohmann::json root) {
    switch (root.type()) {
    case NType::binary:
      return make_json_error(JsonErrorTypes::Invalid,
                             "binary type not supported"sv);
    case NType::discarded:
      return make_json_error(JsonErrorTypes::Invalid,
                             "discarded type not supported"sv);
    default:
      break;
    }

    return NlohValue::create(root);
  }

} // namespace JsonTypedefCodeGen::Reader
