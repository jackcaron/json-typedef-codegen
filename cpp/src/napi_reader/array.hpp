#pragma once

#include "../spec_reader.hpp"

#include <napi.h>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

class NapiArrayIterator : public Specialization::ArrayIterator {
private:
  Napi::Array m_array;
  uint32_t m_index = 0;

public:
  NapiArrayIterator() = delete;
  NapiArrayIterator(const Napi::Array arr) : m_array(arr) {}

  virtual ExpType<JsonValue> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonArrayIterator create(const Napi::Array arr);
};

class NapiArray : public Specialization::Array {
private:
  Napi::Array m_array;

public:
  NapiArray() = delete;
  NapiArray(const Napi::Array arr) : m_array(arr) {}
  ~NapiArray() {}

  virtual JsonArrayIterator begin() const override;

  static JsonArray create(const Napi::Array arr);
};
