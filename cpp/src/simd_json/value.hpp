#pragma once

#include "../spec_reader.hpp"

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

class SimdValue : public Specialization::Value {
private:
  mutable simdjson::ondemand::value m_value;

public:
  SimdValue() = delete;
  SimdValue(simdjson::ondemand::value val) : m_value(val) {}
  ~SimdValue() {}

  virtual JsonTypes get_type() const override;

  virtual ExpType<bool> is_null() const override;
  virtual ExpType<bool> read_bool() const override;
  virtual ExpType<double> read_double() const override;
  virtual ExpType<uint64_t> read_u64() const override;
  virtual ExpType<int64_t> read_i64() const override;
  virtual ExpType<std::string> read_str() const override;
  virtual ExpType<JsonArray> read_array() const override;
  virtual ExpType<JsonObject> read_object() const override;

  static JsonValue create(simdjson::ondemand::value& val);
};

// -------------------------------------------
UnexpJsonError makeJsonError(const simdjson::error_code err_type);

template <typename Type>
ExpType<Type> map_simd_data(simdjson::simdjson_result<Type> data) {
  const auto err_type = data.error();
  [[likely]] if (err_type == simdjson::SUCCESS) {
    return ExpType<Type>(data.value());
  }
  return makeJsonError(err_type);
}
