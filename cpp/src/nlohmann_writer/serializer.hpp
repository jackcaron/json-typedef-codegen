#pragma once

#include "../spec_writer.hpp"
#include "nlohmann/json.hpp"

#include <stack>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;

using NJson = nlohmann::json;

enum class States : uint8_t {
  RootArray,
  Array,

  RootObject,
  Object,
  ObjectKey
};

class NlohSerializer final : public Specialization::AbsSerializer {
private:
  std::stack<States> m_states;
  std::stack<NJson> m_jsons;
  std::stack<std::string> m_keys;

  inline States state() const { return m_states.top(); }
  inline void push_state(const States state) { m_states.emplace(state); }
  inline void pop_state() { m_states.pop(); }

  inline NJson json() { return m_jsons.top(); }
  inline void push_json(NJson js) { m_jsons.emplace(js); }
  inline void pop_json() { m_jsons.pop(); }

  ExpType<void> end_item();

public:
  NlohSerializer() = delete;
  NlohSerializer(NJson root);
  ~NlohSerializer() {}

  virtual ExpType<void> write_null() override;
  virtual ExpType<void> write_bool(const bool b) override;
  virtual ExpType<void> write_double(const double d) override;
  virtual ExpType<void> write_i64(const int64_t i) override;
  virtual ExpType<void> write_u64(const uint64_t u) override;
  virtual ExpType<void> write_str(const std::string_view str) override;

  virtual ExpType<void> start_object() override;
  virtual ExpType<void> write_key(const std::string_view key) override;
  virtual ExpType<void> end_object() override;

  virtual ExpType<void> start_array() override;
  virtual ExpType<void> end_array() override;

  static Serializer create(NJson root);
};
