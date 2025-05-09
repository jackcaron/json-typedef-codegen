#pragma once

#include <memory>

#include "common.hpp"
#include "json_data.hpp"

namespace JsonTypedefCodeGen::Writer {

  class Serializer;

  namespace Specialization {

    class BaseSerializer;
    using SerializerPtr = std::unique_ptr<BaseSerializer>;

    class BaseSerializer {
    protected:
      static Serializer create_serializer(SerializerPtr&& pimpl);

    public:
      virtual ~BaseSerializer();
    };

  } // namespace Specialization

  class Serializer {
  private:
    friend class Specialization::BaseSerializer;

    Specialization::SerializerPtr m_pimpl;
    Serializer(Specialization::SerializerPtr&& pimpl);

  public:
    Serializer() = default;
    Serializer(const Serializer&) = delete;
    Serializer(Serializer&&) = default;
    ~Serializer() {}

    Serializer& operator=(const Serializer&) = delete;
    Serializer& operator=(Serializer&&) = default;

    ExpType<void> write_null();
    ExpType<void> write_bool(const bool b);
    ExpType<void> write_double(const double d);
    ExpType<void> write_i64(const int64_t i);
    ExpType<void> write_u64(const uint64_t u);
    ExpType<void> write_str(const std::string_view str);

    ExpType<void> start_object();
    ExpType<void> write_key(const std::string_view key);
    ExpType<void> end_object();

    ExpType<void> start_array();
    ExpType<void> end_array();

    ExpType<void> write(const Data::JsonArray& arr);
    ExpType<void> write(const Data::JsonObject& obj);
    ExpType<void> write(const Data::JsonValue& val);
  };

} // namespace JsonTypedefCodeGen::Writer
