#pragma once

#include "../spec_reader.hpp"
#include "nlohmann/json.hpp"

// PROBABLY BETA VERSION
// doing a lot of copies for array and object

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

class NlohValue final : public Specialization::Value {
private:
  mutable nlohmann::json m_value;

public:
  NlohValue() = delete;
  NlohValue(const nlohmann::json value) : m_value(value) {}
  ~NlohValue() {}

  virtual JsonTypes get_type() const override;

  virtual ExpType<bool> is_null() const override;
  virtual ExpType<bool> read_bool() const override;
  virtual ExpType<double> read_double() const override;
  virtual ExpType<uint64_t> read_u64() const override;
  virtual ExpType<int64_t> read_i64() const override;
  virtual ExpType<std::string> read_str() const override;
  virtual ExpType<JsonArray> read_array() const override;
  virtual ExpType<JsonObject> read_object() const override;

  virtual NumberType get_number_type() const override;

  static JsonValue create(const nlohmann::json val);
};

// -------------------------------------------