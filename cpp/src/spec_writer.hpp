#pragma once

#include "json_writer.hpp"

// internal specialization for each library
namespace JsonTypedefCodeGen::Writer::Specialization {

  class AbsSerializer : public BaseSerializer {
  public:
    virtual ~AbsSerializer();

    virtual ExpType<bool> write_null() = 0;
    virtual ExpType<bool> write_bool(const bool b) = 0;
    virtual ExpType<bool> write_double(const double d) = 0;
    virtual ExpType<bool> write_i64(const int64_t i) = 0;
    virtual ExpType<bool> write_u64(const uint64_t u) = 0;
    virtual ExpType<bool> write_str(const std::string_view str) = 0;

    virtual ExpType<bool> start_object() = 0;
    virtual ExpType<bool> end_object() = 0;

    virtual ExpType<bool> start_array() = 0;
    virtual ExpType<bool> end_array() = 0;
  };

} // namespace JsonTypedefCodeGen::Writer::Specialization
