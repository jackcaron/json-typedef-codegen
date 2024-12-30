#pragma once

#include "../spec_reader.hpp"

#include "simd_json/simdjson.h"

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

using ObjIter = simdjson::ondemand::object_iterator;
using ObjIterResult = simdjson::simdjson_result<ObjIter>;

class SimdObjectIterator final : public Specialization::ObjectIterator {
private:
  ObjIterResult m_iter;
  ObjIter m_end;

public:
  SimdObjectIterator() = delete;
  SimdObjectIterator(ObjIterResult begin, ObjIter end)
      : m_iter(begin), m_end(end) {}

  virtual ExpType<ObjectIteratorPair> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonObjectIterator create(ObjIterResult begin, ObjIter end);
};

class SimdObject final : public Specialization::Object {
private:
  mutable simdjson::ondemand::object m_object;

public:
  SimdObject() = delete;
  SimdObject(simdjson::ondemand::object obj) : m_object(obj) {}
  ~SimdObject() {}

  virtual JsonObjectIterator begin() const override;

  static JsonObject create(simdjson::ondemand::object obj);
};
