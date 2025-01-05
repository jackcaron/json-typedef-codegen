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
  // ----------------------
  JsonArray::JsonArray()
      : m_array(std::make_shared<Specialization::JsonArray>()) {}
  JsonArray::JsonArray(Specialization::JsonArrayPtr array) : m_array(array) {}
  JsonArray::~JsonArray() {}

  Specialization::JsonArray& JsonArray::internal() { return *m_array; }
  const Specialization::JsonArray& JsonArray::internal() const {
    return *m_array;
  }

  JsonObject::JsonObject()
      : m_object(std::make_shared<Specialization::JsonObject>()) {}
  JsonObject::JsonObject(Specialization::JsonObjectPtr obj) : m_object(obj) {}
  JsonObject::~JsonObject() {}

  Specialization::JsonObject& JsonObject::internal() { return *m_object; }
  const Specialization::JsonObject& JsonObject::internal() const {
    return *m_object;
  }

  // ----------------------
  JsonValue::JsonValue(std::nullptr_t) {}
  JsonValue::JsonValue(const bool b) : m_value(b) {}
  JsonValue::JsonValue(const double d) : m_value(d) {}
  JsonValue::JsonValue(const uint64_t u64) : m_value(u64) {}
  JsonValue::JsonValue(const int64_t i64) : m_value(i64) {}
  JsonValue::JsonValue(const std::string_view str)
      : m_value(std::string(str)) {}
  JsonValue::JsonValue(const std::string& str) : m_value(str) {}
  JsonValue::JsonValue(const JsonArray& array) : m_value(array.m_array) {}
  JsonValue::JsonValue(const JsonObject& object) : m_value(object.m_object) {}
  JsonValue::JsonValue(std::string&& str) : m_value(std::move(str)) {}
  JsonValue::JsonValue(JsonArray&& array) : m_value(std::move(array.m_array)) {}
  JsonValue::JsonValue(JsonObject&& object)
      : m_value(std::move(object.m_object)) {}
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
    m_value = array.m_array;
    return *this;
  }

  JsonValue& JsonValue::operator=(const JsonObject& object) {
    m_value = object.m_object;
    return *this;
  }

  JsonValue& JsonValue::operator=(std::string&& str) {
    m_value = std::move(str);
    return *this;
  }

  JsonValue& JsonValue::operator=(JsonArray&& array) {
    m_value = std::move(array.m_array);
    return *this;
  }

  JsonValue& JsonValue::operator=(JsonObject&& object) {
    m_value = std::move(object.m_object);
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

  bool JsonValue::is_null() const {
    return AllValuesTypes(m_value.index()) == AllValuesTypes::Null;
  }

  std::optional<bool> JsonValue::read_bool() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Bool) {
      return std::get<size_t(AllValuesTypes::Bool)>(m_value);
    }
    return {};
  }

  std::optional<double> JsonValue::read_double() const {
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
    return {};
  }

  std::optional<uint64_t> JsonValue::read_u64() const {
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
    return {};
  }

  std::optional<int64_t> JsonValue::read_i64() const {
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
    return {};
  }

  std::optional<std::string_view> JsonValue::read_str() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::String) {
      return std::get<size_t(AllValuesTypes::String)>(m_value);
    }
    return {};
  }

  std::optional<JsonArray> JsonValue::read_array() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Array) {
      return JsonArray(std::get<size_t(AllValuesTypes::Array)>(m_value));
    }
    return {};
  }

  std::optional<JsonObject> JsonValue::read_object() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Object) {
      return JsonObject(std::get<size_t(AllValuesTypes::Object)>(m_value));
    }
    return {};
  }

} // namespace JsonTypedefCodeGen::Data
