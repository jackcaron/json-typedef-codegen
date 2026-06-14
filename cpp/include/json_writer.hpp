#pragma once

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
      virtual ~BaseSerializer() noexcept;
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
    ~Serializer();

    Serializer& operator=(const Serializer&) = delete;
    Serializer& operator=(Serializer&&) = default;

    [[nodiscard]] ExpType<void> close();

    [[nodiscard]] ExpType<void> write_null();
    [[nodiscard]] ExpType<void> write_bool(const bool b);
    [[nodiscard]] ExpType<void> write_double(const double d);
    [[nodiscard]] ExpType<void> write_i64(const int64_t i);
    [[nodiscard]] ExpType<void> write_u64(const uint64_t u);
    [[nodiscard]] ExpType<void> write_str(const std::string_view str);

    [[nodiscard]] ExpType<void> start_object();
    [[nodiscard]] ExpType<void> write_key(const std::string_view key);
    [[nodiscard]] ExpType<void> end_object();

    [[nodiscard]] ExpType<void> start_array();
    [[nodiscard]] ExpType<void> end_array();

    [[nodiscard]] ExpType<void> write(const Data::JsonArray& arr);
    [[nodiscard]] ExpType<void> write(const Data::JsonObject& obj);
    [[nodiscard]] ExpType<void> write(const Data::JsonValue& val);
  };

} // namespace JsonTypedefCodeGen::Writer
