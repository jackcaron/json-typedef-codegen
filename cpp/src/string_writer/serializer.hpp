#pragma once

#include "../../include/string_serializer.hpp"
#include "../spec_writer.hpp"

#include <stack>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;

class InternalStringSerializer final : public Specialization::AbsSerializer {
private:
  StringSerializer* m_str_ser = nullptr;

public:
  InternalStringSerializer() = delete;
  InternalStringSerializer(StringSerializer& str_ser) : m_str_ser(&str_ser) {}
  ~InternalStringSerializer();

  virtual ExpType<void> close() override;

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

  static Serializer create(StringSerializer& str_ser);
};