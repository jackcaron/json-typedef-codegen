#pragma once

#include "../spec_reader.hpp"

#include <napi.h>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

class NapiObjectIterator : public Specialization::ObjectIterator {
private:
  Napi::Object m_object;
  Napi::Array m_keys;
  uint32_t m_index = 0;

public:
  NapiObjectIterator() = delete;
  NapiObjectIterator(Napi::Object object);

  virtual ExpType<ObjectIteratorPair> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonObjectIterator create(Napi::Object object);
};

class NapiObject : public Specialization::Object {
private:
  Napi::Object m_object;

public:
  NapiObject() = delete;
  NapiObject(const Napi::Object obj) : m_object(obj) {}
  ~NapiObject() {}

  virtual JsonObjectIterator begin() const override;

  static JsonObject create(const Napi::Object obj);
};
