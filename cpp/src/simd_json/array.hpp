#pragma once

#include "../spec_reader.hpp"

#include "simd_json/simdjson.h"

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

using ArrIter = simdjson::ondemand::array_iterator;
using ArrIterResult = simdjson::simdjson_result<ArrIter>;

class SimdArrayIterator final : public Specialization::ArrayIterator {
private:
  ArrIterResult m_iter;
  ArrIter m_end;

public:
  SimdArrayIterator() = delete;
  SimdArrayIterator(ArrIterResult begin, ArrIter end)
      : m_iter(begin), m_end(end) {}

  virtual ExpType<JsonValue> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonArrayIterator create(ArrIterResult begin, ArrIter end);
};

class SimdArray final : public Specialization::Array {
private:
  mutable simdjson::ondemand::array m_array;

public:
  SimdArray() = delete;
  SimdArray(simdjson::ondemand::array arr) : m_array(arr) {}
  ~SimdArray() {}

  virtual JsonArrayIterator begin() const override;

  static JsonArray create(simdjson::ondemand::array arr);
};
