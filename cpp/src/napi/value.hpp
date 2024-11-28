#pragma once

#include "../spec_reader.hpp"

#include <napi.h>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

class NapiValue : public Specialization::Value {
private:
  Napi::Value m_value;

public:
  NapiValue() = delete;
  NapiValue(const Napi::Value val) : m_value(val) {}
  ~NapiValue() {}

  virtual JsonTypes get_type() const override;

  virtual ExpType<bool> is_null() const override;
  virtual ExpType<bool> read_bool() const override;
  virtual ExpType<double> read_double() const override;
  virtual ExpType<uint64_t> read_u64() const override;
  virtual ExpType<int64_t> read_i64() const override;
  virtual ExpType<std::string> read_str() const override;
  virtual ExpType<JsonArray> read_array() const override;
  virtual ExpType<JsonObject> read_object() const override;

  static JsonValue create(const Napi::Value val);
};

// -------------------------------------------

// -------------------------------------------
UnexpJsonError makeJsonError(const napi_status err_type);
