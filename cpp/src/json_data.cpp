#include "json_data.hpp"

#include "internal.hpp"

using namespace JsonTypedefCodeGen;

namespace {

  enum class AllValuesTypes : size_t {
    Null = 0,
    Bool,
    Double,
    U64,
    I64,
    String,
    Array,
    Object
  };

  JsonTypes map_variant(const AllValuesTypes val) {
    switch (val) {
    case AllValuesTypes::Null:
      return JsonTypes::Null;
    case AllValuesTypes::Bool:
      return JsonTypes::Bool;
    case AllValuesTypes::Double:
    case AllValuesTypes::U64:
    case AllValuesTypes::I64:
      return JsonTypes::Number;
    case AllValuesTypes::String:
      return JsonTypes::String;
    case AllValuesTypes::Array:
      return JsonTypes::Array;
    case AllValuesTypes::Object:
      return JsonTypes::Object;
    default:
      break;
    }
    return JsonTypes::Invalid;
  }

} // namespace

namespace JsonTypedefCodeGen::Data {

  JsonValue::JsonValue(std::nullptr_t) {}
  JsonValue::JsonValue(const bool b) : m_value(b) {}
  JsonValue::JsonValue(const double d) : m_value(d) {}
  JsonValue::JsonValue(const uint64_t u64) : m_value(u64) {}
  JsonValue::JsonValue(const int64_t i64) : m_value(i64) {}
  JsonValue::JsonValue(const std::string_view str)
      : m_value(std::string(str)) {}
  JsonValue::JsonValue(const std::string& str) : m_value(str) {}
  JsonValue::JsonValue(const JsonArray& array) : m_value(array) {}
  JsonValue::JsonValue(const JsonObject& object) : m_value(object) {}
  JsonValue::JsonValue(std::string&& str) : m_value(std::move(str)) {}
  JsonValue::JsonValue(JsonArray&& array) : m_value(std::move(array)) {}
  JsonValue::JsonValue(JsonObject&& object) : m_value(std::move(object)) {}
  JsonValue::~JsonValue() {}

  //

  JsonValue& JsonValue::operator=(std::nullptr_t) {
    m_value = nullptr;
    return *this;
  }

  JsonValue& JsonValue::operator=(const bool b) {
    m_value = b;
    return *this;
  }

  JsonValue& JsonValue::operator=(const double d) {
    m_value = d;
    return *this;
  }

  JsonValue& JsonValue::operator=(const uint64_t u64) {
    m_value = u64;
    return *this;
  }

  JsonValue& JsonValue::operator=(const int64_t i64) {
    m_value = i64;
    return *this;
  }

  JsonValue& JsonValue::operator=(const std::string_view str) {
    m_value = std::string(str);
    return *this;
  }

  JsonValue& JsonValue::operator=(const std::string& str) {
    m_value = str;
    return *this;
  }

  JsonValue& JsonValue::operator=(const JsonArray& array) {
    m_value = array;
    return *this;
  }

  JsonValue& JsonValue::operator=(const JsonObject& object) {
    m_value = object;
    return *this;
  }

  JsonValue& JsonValue::operator=(std::string&& str) {
    m_value = std::move(str);
    return *this;
  }

  JsonValue& JsonValue::operator=(JsonArray&& array) {
    m_value = std::move(array);
    return *this;
  }

  JsonValue& JsonValue::operator=(JsonObject&& object) {
    m_value = std::move(object);
    return *this;
  }

  //

  JsonTypes JsonValue::get_type() const {
    return map_variant(AllValuesTypes(m_value.index()));
  }

  NumberType JsonValue::get_number_type() const {
    switch (AllValuesTypes(m_value.index())) {
    case AllValuesTypes::Double:
      return NumberType::Double;
    case AllValuesTypes::I64:
      return NumberType::I64;
    case AllValuesTypes::U64:
      return NumberType::U64;
    default:
      break;
    }
    return NumberType::NaN;
  }

  ExpType<bool> JsonValue::is_null() const {
    return AllValuesTypes(m_value.index()) == AllValuesTypes::Null;
  }

  ExpType<bool> JsonValue::read_bool() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Bool) {
      return std::get<size_t(AllValuesTypes::Bool)>(m_value);
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<double> JsonValue::read_double() const {
    switch (AllValuesTypes(m_value.index())) {
    case AllValuesTypes::Double:
      return std::get<size_t(AllValuesTypes::Double)>(m_value);
    case AllValuesTypes::I64:
      return std::get<size_t(AllValuesTypes::I64)>(m_value);
    case AllValuesTypes::U64:
      return std::get<size_t(AllValuesTypes::U64)>(m_value);
    default:
      break;
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<uint64_t> JsonValue::read_u64() const {
    switch (AllValuesTypes(m_value.index())) {
    case AllValuesTypes::Double:
      return std::get<size_t(AllValuesTypes::Double)>(m_value);
    case AllValuesTypes::I64:
      return std::get<size_t(AllValuesTypes::I64)>(m_value);
    case AllValuesTypes::U64:
      return std::get<size_t(AllValuesTypes::U64)>(m_value);
    default:
      break;
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<int64_t> JsonValue::read_i64() const {
    switch (AllValuesTypes(m_value.index())) {
    case AllValuesTypes::Double:
      return std::get<size_t(AllValuesTypes::Double)>(m_value);
    case AllValuesTypes::I64:
      return std::get<size_t(AllValuesTypes::I64)>(m_value);
    case AllValuesTypes::U64:
      return std::get<size_t(AllValuesTypes::U64)>(m_value);
    default:
      break;
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<std::string_view> JsonValue::read_str() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::String) {
      return std::get<size_t(AllValuesTypes::String)>(m_value);
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<const std::reference_wrapper<JsonArray>>
  JsonValue::read_array() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Array) {
      auto ref = std::get<size_t(AllValuesTypes::Array)>(m_value);
      return std::reference_wrapper<JsonArray>(ref);
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

  ExpType<const std::reference_wrapper<JsonObject>>
  JsonValue::read_object() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Object) {
      auto ref = std::get<size_t(AllValuesTypes::Object)>(m_value);
      return std::reference_wrapper<JsonObject>(ref);
    }
    return makeJsonError(JsonErrorTypes::WrongType);
  }

} // namespace JsonTypedefCodeGen::Data
