#pragma once

#include "../spec_writer.hpp"

#include <napi.h>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;

class NapiSerializer final : public Specialization::StateBaseSerializer {
private:
  Napi::Value m_root;
  std::stack<Napi::Value> m_jsons;

  inline Napi::Value& json() { return m_jsons.top(); }
  inline void push_json(Napi::Value js) { m_jsons.emplace(js); }
  inline void pop_json() { m_jsons.pop(); }

  ExpType<void> end_item();

public:
  NapiSerializer() = delete;
  NapiSerializer(Napi::Value& root);
  ~NapiSerializer() {}

  virtual ExpType<void> close() override;

  virtual ExpType<void> write_null() override;
  virtual ExpType<void> write_bool(const bool b) override;
  virtual ExpType<void> write_double(const double d) override;
  virtual ExpType<void> write_i64(const int64_t i) override;
  virtual ExpType<void> write_u64(const uint64_t u) override;
  virtual ExpType<void> write_str(const std::string_view str) override;

  virtual ExpType<void> start_object() override;
  virtual ExpType<void> end_object() override;

  virtual ExpType<void> start_array() override;
  virtual ExpType<void> end_array() override;

  static Serializer create(Napi::Value& root);
};
