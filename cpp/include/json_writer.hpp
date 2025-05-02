#pragma once

#include <memory>

#include "common.hpp"

namespace JsonTypedefCodeGen::Writer {

  class Serializer;

  namespace Specialization {

    class BaseSerializer;
    using SerializerPtr = std::unique_ptr<BaseSerializer>;

    class BaseSerializer {
    protected:
      static Serializer create(SerializerPtr&& pimpl);

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

    ExpType<bool> write_null();
    ExpType<bool> write_bool(const bool b);
    ExpType<bool> write_double(const double d);
    ExpType<bool> write_i64(const int64_t i);
    ExpType<bool> write_u64(const uint64_t u);
    ExpType<bool> write_str(const std::string_view str);

    ExpType<bool> start_object();
    ExpType<bool> end_object();

    ExpType<bool> start_array();
    ExpType<bool> end_array();
  };

} // namespace JsonTypedefCodeGen::Writer
