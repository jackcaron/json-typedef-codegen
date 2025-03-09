#pragma once

#include "../spec_reader.hpp"
#include "nlohmann/json.hpp"

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Reader;

using NlohMap = nlohmann::json::object_t;
using NlohMapIter = NlohMap::iterator;

class NlohObjectIterator final : public Specialization::ObjectIterator {
private:
  NlohMapIter m_iter, m_end;

public:
  NlohObjectIterator() = delete;
  NlohObjectIterator(NlohMapIter begin, NlohMapIter end)
      : m_iter(begin), m_end(end) {}

  virtual ExpType<ObjectIteratorPair> get() const override;
  virtual void next() override;
  virtual bool done() const override;

  static JsonObjectIterator create(NlohMapIter begin, NlohMapIter end);
};

class NlohObject final : public Specialization::Object {
private:
  mutable NlohMap m_object;

public:
  NlohObject() = delete;
  NlohObject(const NlohMap obj) : m_object(obj) {}
  ~NlohObject() {}

  virtual JsonObjectIterator begin() const override;

  static JsonObject create(const NlohMap obj);
};
