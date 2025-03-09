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
  DLL_PUBLIC JsonArray::JsonArray()
      : m_array(std::make_shared<Specialization::JsonArray>()) {}
  DLL_PUBLIC JsonArray::JsonArray(Specialization::JsonArrayPtr array)
      : m_array(array) {}
  DLL_PUBLIC JsonArray::~JsonArray() {}

  DLL_PUBLIC Specialization::JsonArray& JsonArray::internal() {
    return *m_array;
  }
  DLL_PUBLIC const Specialization::JsonArray& JsonArray::internal() const {
    return *m_array;
  }

  DLL_PUBLIC JsonObject::JsonObject()
      : m_object(std::make_shared<Specialization::JsonObject>()) {}
  DLL_PUBLIC JsonObject::JsonObject(Specialization::JsonObjectPtr obj)
      : m_object(obj) {}
  DLL_PUBLIC JsonObject::~JsonObject() {}

  DLL_PUBLIC Specialization::JsonObject& JsonObject::internal() {
    return *m_object;
  }
  DLL_PUBLIC const Specialization::JsonObject& JsonObject::internal() const {
    return *m_object;
  }

  // ----------------------
  DLL_PUBLIC JsonValue::JsonValue(std::nullptr_t) {}
  DLL_PUBLIC JsonValue::JsonValue(const bool b) : m_value(b) {}
  DLL_PUBLIC JsonValue::JsonValue(const double d) : m_value(d) {}
  DLL_PUBLIC JsonValue::JsonValue(const uint64_t u64) : m_value(u64) {}
  DLL_PUBLIC JsonValue::JsonValue(const int64_t i64) : m_value(i64) {}
  DLL_PUBLIC JsonValue::JsonValue(const std::string_view str)
      : m_value(std::string(str)) {}
  DLL_PUBLIC JsonValue::JsonValue(const std::string& str) : m_value(str) {}
  DLL_PUBLIC JsonValue::JsonValue(const JsonArray& array)
      : m_value(array.m_array) {}
  DLL_PUBLIC JsonValue::JsonValue(const JsonObject& object)
      : m_value(object.m_object) {}
  DLL_PUBLIC JsonValue::JsonValue(std::string&& str)
      : m_value(std::move(str)) {}
  DLL_PUBLIC JsonValue::JsonValue(JsonArray&& array)
      : m_value(std::move(array.m_array)) {}
  DLL_PUBLIC JsonValue::JsonValue(JsonObject&& object)
      : m_value(std::move(object.m_object)) {}
  DLL_PUBLIC JsonValue::~JsonValue() {}

  //

  DLL_PUBLIC JsonValue& JsonValue::operator=(std::nullptr_t) {
    m_value = nullptr;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const bool b) {
    m_value = b;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const double d) {
    m_value = d;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const uint64_t u64) {
    m_value = u64;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const int64_t i64) {
    m_value = i64;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const std::string_view str) {
    m_value = std::string(str);
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const std::string& str) {
    m_value = str;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const JsonArray& array) {
    m_value = array.m_array;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(const JsonObject& object) {
    m_value = object.m_object;
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(std::string&& str) {
    m_value = std::move(str);
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(JsonArray&& array) {
    m_value = std::move(array.m_array);
    return *this;
  }

  DLL_PUBLIC JsonValue& JsonValue::operator=(JsonObject&& object) {
    m_value = std::move(object.m_object);
    return *this;
  }

  //

  DLL_PUBLIC JsonTypes JsonValue::get_type() const {
    return map_variant(AllValuesTypes(m_value.index()));
  }

  DLL_PUBLIC NumberType JsonValue::get_number_type() const {
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

  DLL_PUBLIC bool JsonValue::is_null() const {
    return AllValuesTypes(m_value.index()) == AllValuesTypes::Null;
  }

  DLL_PUBLIC std::optional<bool> JsonValue::read_bool() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Bool) {
      return std::get<size_t(AllValuesTypes::Bool)>(m_value);
    }
    return {};
  }

  DLL_PUBLIC std::optional<double> JsonValue::read_double() const {
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

  DLL_PUBLIC std::optional<uint64_t> JsonValue::read_u64() const {
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

  DLL_PUBLIC std::optional<int64_t> JsonValue::read_i64() const {
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

  DLL_PUBLIC std::optional<std::string_view> JsonValue::read_str() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::String) {
      return std::get<size_t(AllValuesTypes::String)>(m_value);
    }
    return {};
  }

  DLL_PUBLIC std::optional<JsonArray> JsonValue::read_array() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Array) {
      return JsonArray(std::get<size_t(AllValuesTypes::Array)>(m_value));
    }
    return {};
  }

  DLL_PUBLIC std::optional<JsonObject> JsonValue::read_object() const {
    if (AllValuesTypes(m_value.index()) == AllValuesTypes::Object) {
      return JsonObject(std::get<size_t(AllValuesTypes::Object)>(m_value));
    }
    return {};
  }

  DLL_PUBLIC ExpType<void> json_array_for_each(const JsonArray& array,
                                               ArrayForEachFn cb) {
    for (auto item : array) {
      if (auto exp = cb(item); !exp.has_value()) {
        return UnexpJsonError(exp.error());
      }
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> json_array_for_each(const JsonValue& value,
                                               ArrayForEachFn cb) {
    if (auto opt_arr = value.read_array(); opt_arr.has_value()) {
      return json_array_for_each(*opt_arr, cb);
    }
    return make_json_error(JsonErrorTypes::Invalid,
                           std::string_view("expected an array"));
  }

  DLL_PUBLIC ExpType<void> json_object_for_each(const JsonObject& object,
                                                ObjectForEachFn cb) {
    for (auto& [key, val] : object) {
      if (auto exp = cb(key, val); !exp.has_value()) {
        return UnexpJsonError(exp.error());
      }
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> json_object_for_each(const JsonValue& value,
                                                ObjectForEachFn cb) {
    if (auto opt_obj = value.read_object(); opt_obj.has_value()) {
      return json_object_for_each(*opt_obj, cb);
    }
    return make_json_error(JsonErrorTypes::Invalid,
                           std::string_view("expected an object"));
  }

} // namespace JsonTypedefCodeGen::Data
