#pragma once

#ifdef IMPL_SERIALIZE

#include "json_data.hpp"
#include "json_writer.hpp"

#include <memory>

// utility functions for the serialized generated code

namespace JsonTypedefCodeGen::Serialize {

  namespace JDt = JsonTypedefCodeGen::Data;
  namespace JWt = JsonTypedefCodeGen::Writer;
  using strview = std::string_view;

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const std::nullptr_t) {
    return serializer.write_null();
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const bool value) {
    return serializer.write_bool(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const int8_t value) {
    return serializer.write_i64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const int16_t value) {
    return serializer.write_i64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const int32_t value) {
    return serializer.write_i64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const int64_t value) {
    return serializer.write_i64(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const uint8_t value) {
    return serializer.write_u64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const uint16_t value) {
    return serializer.write_u64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const uint32_t value) {
    return serializer.write_u64(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const uint64_t value) {
    return serializer.write_u64(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const float value) {
    return serializer.write_double(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const double value) {
    return serializer.write_double(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const strview value) {
    return serializer.write_str(value);
  }

  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const JDt::JsonArray value) {
    return serializer.write(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const JDt::JsonObject value) {
    return serializer.write(value);
  }
  inline ExpType<void> serialize(JWt::Serializer& serializer,
                                 const JDt::JsonValue value) {
    return serializer.write(value);
  }

#define SHORT_EXP(expr)                                                        \
  if (ExpType<void> exp = (expr); !exp.has_value()) {                          \
    return exp;                                                                \
  }

  template <typename Type>
  ExpType<void> serialize(JWt::Serializer& serializer,
                          const std::vector<Type>& values) {
    //
    SHORT_EXP(serializer.start_array());
    for (const auto& item : values) {
      SHORT_EXP(serialize(serializer, item));
    }
    return serializer.end_array();
  }

  template <typename Type>
  ExpType<void> serialize(JWt::Serializer& serializer,
                          const JsonMap<Type>& values) {
    //
    SHORT_EXP(serializer.start_object());
    for (const auto& [key, item] : values) {
      SHORT_EXP(serializer.write_key(key));
      SHORT_EXP(serialize(serializer, item));
    }
    return serializer.end_object();
  }

  template <typename Nullable>
  ExpType<void> serialize(JWt::Serializer& serializer,
                          const std::unique_ptr<Nullable>& value) {
    if (!!value) {
      return serialize(serializer, *value);
    }
    return serializer.write_null();
  }

#undef SHORT_EXP

} // namespace JsonTypedefCodeGen::Serialize

#endif
