#pragma once

#include "../spec_reader.hpp"
#include "nlohmann/json.hpp"

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

using NlohVector = nlohmann::json::array_t;
using NlohVectorIter = NlohVector::iterator;

class NlohArrayIterator final : public Specialization::ArrayIterator {
private:
  NlohVectorIter m_iter, m_end;

public:
  NlohArrayIterator() = delete;
  NlohArrayIterator(NlohVectorIter begin, NlohVectorIter end)
      : m_iter(begin), m_end(end) {}

  virtual ExpType<JsonValue> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonArrayIterator create(NlohVectorIter begin, NlohVectorIter end);
};

class NlohArray final : public Specialization::Array {
private:
  mutable NlohVector m_array;

public:
  NlohArray() = delete;
  NlohArray(const NlohVector arr) : m_array(arr) {}
  ~NlohArray() {}

  virtual JsonArrayIterator begin() const override;

  static JsonArray create(const NlohArray arr);
};
