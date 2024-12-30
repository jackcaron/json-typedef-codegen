#pragma once

#include "common.hpp"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace JsonTypedefCodeGen::Data {

  class JsonValue;

  using JsonArray = std::vector<JsonValue>;
  using JsonObject = std::unordered_map<std::string, JsonValue>;

  class JsonValue {
  private:
    using AllValues = std::variant<std::nullptr_t, bool, double, uint64_t,
                                   int64_t, std::string, JsonArray, JsonObject>;

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

    ExpType<bool> is_null() const;
    ExpType<bool> read_bool() const;
    ExpType<double> read_double() const;
    ExpType<uint64_t> read_u64() const;
    ExpType<int64_t> read_i64() const;
    ExpType<std::string_view> read_str() const;
    ExpType<const std::reference_wrapper<JsonArray>> read_array() const;
    ExpType<const std::reference_wrapper<JsonObject>> read_object() const;
  };

} // namespace JsonTypedefCodeGen::Data
