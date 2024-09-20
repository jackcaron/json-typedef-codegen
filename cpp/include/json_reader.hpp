#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <string>

namespace JsonTypedefCodeGen {

// probably not the best place
enum class JsonTypes {
  Null,
  Bool,
  Number,
  Array,
  Object,
  String
};

// place holder
class JsonError {};

template <typename Type> using ExpType = std::expected<Type, JsonError>;

class JsonArray;
class JsonObject;
class JsonValue;
using JsonArrayPtr = std::unique_ptr<JsonArray>;
using JsonObjectPtr = std::unique_ptr<JsonObject>;
using JsonValuePtr = std::unique_ptr<JsonValue>;

class JsonArray {
public:
  virtual ~JsonArray() {}

  // for "simdjson::ondemand", data doesn't persist and must be consumed when
  // given
  virtual bool must_use_iterators() = 0;

  virtual ExpType<uint32_t> get_size() = 0;
  virtual ExpType<JsonValuePtr> get_element(const uint32_t idx) = 0;
  // iterators
};

class JsonObject {
public:
  virtual ~JsonObject() {}

  // for "simdjson::ondemand", data doesn't persist and must be consumed when
  // given
  virtual bool must_use_iterators() = 0;

  //
  // get
  // iterators
};

class JsonValue {
public:
  virtual ~JsonValue() {}

  virtual JsonTypes get_type() = 0;

  virtual ExpType<bool> is_null() = 0;
  virtual ExpType<bool> read_bool() = 0;
  virtual ExpType<double> read_double() = 0;
  virtual ExpType<uint32_t> read_u32() = 0;
  virtual ExpType<int32_t> read_i32() = 0;
  virtual ExpType<std::string> read_str() = 0;
  virtual ExpType<JsonArrayPtr> read_array() = 0;
  virtual ExpType<JsonObjectPtr> read_object() = 0;
};

} // namespace JsonTypedefCodeGen
