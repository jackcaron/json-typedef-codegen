#pragma once

#include "json_writer.hpp"

// internal specialization for each library
namespace JsonTypedefCodeGen::Writer::Specialization {

  class AbsSerializer : public BaseSerializer {
  public:
    virtual ~AbsSerializer();

    virtual ExpType<void> close() = 0;

    virtual ExpType<void> write_null() = 0;
    virtual ExpType<void> write_bool(const bool b) = 0;
    virtual ExpType<void> write_double(const double d) = 0;
    virtual ExpType<void> write_i64(const int64_t i) = 0;
    virtual ExpType<void> write_u64(const uint64_t u) = 0;
    virtual ExpType<void> write_str(const std::string_view str) = 0;

    virtual ExpType<void> start_object() = 0;
    virtual ExpType<void> write_key(const std::string_view key) = 0;
    virtual ExpType<void> end_object() = 0;

    virtual ExpType<void> start_array() = 0;
    virtual ExpType<void> end_array() = 0;

    ExpType<ExpType<void>> _write_number(const Data::JsonValue& val);
    ExpType<ExpType<void>> _write(const Data::JsonValue& val);

    ExpType<void> write(const Data::JsonArray& arr);
    ExpType<void> write(const Data::JsonObject& obj);
    ExpType<void> write(const Data::JsonValue& val);
  };

} // namespace JsonTypedefCodeGen::Writer::Specialization
