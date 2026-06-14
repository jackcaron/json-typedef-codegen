#pragma once

#ifdef IMPL_SERIALIZE

#include "headers.hpp"
#include "json_data.hpp"
#include "json_writer.hpp"

// utility functions for the serialized generated code

namespace JsonTypedefCodeGen::Serialize {

  namespace JDt = JsonTypedefCodeGen::Data;
  namespace JWt = JsonTypedefCodeGen::Writer;
  using strview = std::string_view;

  template <typename Type> struct Serialize;

  template <> struct Serialize<std::nullptr_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const std::nullptr_t) {
      return serializer.write_null();
    }
  };

  template <> struct Serialize<bool> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const bool value) {
      return serializer.write_bool(value);
    }
  };

  template <> struct Serialize<int8_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const int8_t value) {
      return serializer.write_i64(value);
    }
  };
  template <> struct Serialize<int16_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const int16_t value) {
      return serializer.write_i64(value);
    }
  };
  template <> struct Serialize<int32_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const int32_t value) {
      return serializer.write_i64(value);
    }
  };
  template <> struct Serialize<int64_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const int64_t value) {
      return serializer.write_i64(value);
    }
  };

  template <> struct Serialize<uint8_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const uint8_t value) {
      return serializer.write_u64(value);
    }
  };
  template <> struct Serialize<uint16_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const uint16_t value) {
      return serializer.write_u64(value);
    }
  };
  template <> struct Serialize<uint32_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const uint32_t value) {
      return serializer.write_u64(value);
    }
  };
  template <> struct Serialize<uint64_t> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const uint64_t value) {
      return serializer.write_u64(value);
    }
  };

  template <> struct Serialize<float> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const float value) {
      return serializer.write_double(value);
    }
  };
  template <> struct Serialize<double> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const double value) {
      return serializer.write_double(value);
    }
  };

  template <> struct Serialize<strview> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const strview value) {
      return serializer.write_str(value);
    }
  };

  template <> struct Serialize<std::string> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const strview value) {
      return serializer.write_str(value);
    }
  };

  template <> struct Serialize<JDt::JsonArray> {
    [[nodiscard]] static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                                        const JDt::JsonArray value) {
      return serializer.write(value);
    }
  };
  template <> struct Serialize<JDt::JsonObject> {
    static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                          const JDt::JsonObject value) {
      return serializer.write(value);
    }
  };
  template <> struct Serialize<JDt::JsonValue> {
    static inline ExpType<void> serialize(JWt::Serializer& serializer,
                                          const JDt::JsonValue value) {
      return serializer.write(value);
    }
  };

  // ------------------------------------------
#define SHORT_EXP(expr)                                                             \
  if (ExpType<void> exp = (expr); !exp.has_value()) {                               \
    return exp;                                                                     \
  }

  template <typename Type>
  [[nodiscard]] static inline ExpType<void>
  serialize_key_value(JWt::Serializer& serializer, const std::string_view key,
                      const Type& value) {
    //
    SHORT_EXP(serializer.write_key(key));
    return Serialize<Type>::serialize(serializer, value);
  }

  // ------------------------------------------
  template <typename Type> struct Serialize<std::vector<Type>> {
    using SubType = Serialize<Type>;
    [[nodiscard]] static inline ExpType<void>
    serialize(JWt::Serializer& serializer, const std::vector<Type>& values) {
      SHORT_EXP(serializer.start_array());
      for (const auto& item : values) {
        SHORT_EXP(SubType::serialize(serializer, item));
      }
      return serializer.end_array();
    }
  };

  template <typename Type> struct Serialize<JsonMap<Type>> {
    using SubType = Serialize<Type>;
    [[nodiscard]] static inline ExpType<void>
    serialize(JWt::Serializer& serializer, const JsonMap<Type>& values) {
      SHORT_EXP(serializer.start_object());
      for (const auto& [key, item] : values) {
        SHORT_EXP(serialize_key_value(serializer, key, item));
      }
      return serializer.end_object();
    }
  };

  template <typename Nullable> struct Serialize<std::unique_ptr<Nullable>> {
    using SubNull = Serialize<Nullable>;
    [[nodiscard]] static inline ExpType<void>
    serialize(JWt::Serializer& serializer, const std::unique_ptr<Nullable>& value) {
      if (!!value) {
        return SubNull::serialize(serializer, *value);
      }
      return serializer.write_null();
    }
  };

#undef SHORT_EXP

} // namespace JsonTypedefCodeGen::Serialize

#endif
