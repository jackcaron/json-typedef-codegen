#pragma once

#include "common.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

// Neutral JSON representation
// To get full access to the JsonArray and JsonObject, use
// the "internal" function to access its internal data structure

namespace JsonTypedefCodeGen::Data {

  class JsonValue;
  class JsonArray;
  class JsonObject;

  namespace Specialization {
    using JsonArray = std::vector<JsonValue>;
    using JsonObject = std::map<std::string, JsonValue>;

    using JsonArrayPtr = std::shared_ptr<JsonArray>;
    using JsonObjectPtr = std::shared_ptr<JsonObject>;
  } // namespace Specialization

  // Iterator Utils, stop at the first error
  using ArrayForEachFn = std::function<ExpType<void>(const JsonValue&)>;
  using ObjectForEachFn =
      std::function<ExpType<void>(const std::string_view, const JsonValue&)>;

  ExpType<void> json_array_for_each(const JsonArray& array, ArrayForEachFn cb);
  ExpType<void> json_array_for_each(const JsonValue& value, ArrayForEachFn cb);

  ExpType<void> json_object_for_each(const JsonObject& object,
                                     ObjectForEachFn cb);
  ExpType<void> json_object_for_each(const JsonValue& value,
                                     ObjectForEachFn cb);

  class JsonArray {
  private:
    friend class JsonValue;

    Specialization::JsonArrayPtr m_array;

    JsonArray(Specialization::JsonArrayPtr array);

  public:
    JsonArray();
    JsonArray(const JsonArray&) = default;
    JsonArray(JsonArray&&) = default;
    ~JsonArray();

    JsonArray& operator=(const JsonArray&) = default;
    JsonArray& operator=(JsonArray&&) = default;

    inline auto begin() { return m_array->begin(); }
    inline auto end() { return m_array->end(); }
    inline auto begin() const { return m_array->begin(); }
    inline auto end() const { return m_array->end(); }
    inline auto empty() const { return m_array->empty(); }
    inline auto size() const { return m_array->size(); }

    Specialization::JsonArray& internal();
    const Specialization::JsonArray& internal() const;

    inline ExpType<void> for_each(ArrayForEachFn cb) const {
      return json_array_for_each(*this, cb);
    }
  };

  class JsonObject {
  private:
    friend class JsonValue;

    Specialization::JsonObjectPtr m_object;

    JsonObject(Specialization::JsonObjectPtr obj);

  public:
    JsonObject();
    JsonObject(const JsonObject&) = default;
    JsonObject(JsonObject&&) = default;
    ~JsonObject();

    JsonObject& operator=(const JsonObject&) = default;
    JsonObject& operator=(JsonObject&&) = default;

    inline auto begin() { return m_object->begin(); }
    inline auto end() { return m_object->end(); }
    inline auto begin() const { return m_object->begin(); }
    inline auto end() const { return m_object->end(); }
    inline auto empty() const { return m_object->empty(); }
    inline auto size() const { return m_object->size(); }

    Specialization::JsonObject& internal();
    const Specialization::JsonObject& internal() const;

    inline ExpType<void> for_each(ObjectForEachFn cb) const {
      return json_object_for_each(*this, cb);
    }
  };

  class JsonValue {
  private:
    using AllValues =
        std::variant<std::nullptr_t, bool, double, uint64_t, int64_t,
                     std::string, Specialization::JsonArrayPtr,
                     Specialization::JsonObjectPtr>;

    AllValues m_value;

  public:
    JsonValue() = default; // null
    JsonValue(const JsonValue&) = default;
    JsonValue(JsonValue&&) = default;
    JsonValue(std::nullptr_t);
    JsonValue(const bool b);
    JsonValue(const double d);
    JsonValue(const uint64_t u64);
    JsonValue(const int64_t i64);
    JsonValue(const std::string_view str);
    JsonValue(const std::string& str);
    JsonValue(const JsonArray& array);
    JsonValue(const JsonObject& object);
    JsonValue(std::string&& str);
    JsonValue(JsonArray&& array);
    JsonValue(JsonObject&& object);
    ~JsonValue();

    JsonValue& operator=(const JsonValue&) = default;
    JsonValue& operator=(JsonValue&&) = default;
    JsonValue& operator=(std::nullptr_t);
    JsonValue& operator=(const bool b);
    JsonValue& operator=(const double d);
    JsonValue& operator=(const uint64_t u64);
    JsonValue& operator=(const int64_t i64);
    JsonValue& operator=(const std::string_view str);
    JsonValue& operator=(const std::string& str);
    JsonValue& operator=(const JsonArray& array);
    JsonValue& operator=(const JsonObject& object);
    JsonValue& operator=(std::string&& str);
    JsonValue& operator=(JsonArray&& array);
    JsonValue& operator=(JsonObject&& object);

    JsonTypes get_type() const;
    NumberType get_number_type() const;

    bool is_null() const;
    std::optional<bool> read_bool() const;
    std::optional<double> read_double() const;
    std::optional<uint64_t> read_u64() const;
    std::optional<int64_t> read_i64() const;
    std::optional<std::string_view> read_str() const;
    std::optional<JsonArray> read_array() const;
    std::optional<JsonObject> read_object() const;
  };

} // namespace JsonTypedefCodeGen::Data
