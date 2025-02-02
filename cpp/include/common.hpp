#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace JsonTypedefCodeGen {

  enum class NumberType {
    Double,
    U64,
    I64,
    NaN
  };

  // --------------------------------------
  enum class JsonTypes {
    Null,
    Bool,
    Number,
    Array,
    Object,
    String,
    Invalid
  };

  std::string_view get_name(const JsonTypes type);

  // --------------------------------------
  enum class JsonErrorTypes {
    Invalid,   // there's nothing
    WrongType, // expecting X, but it's something else
    Number,    // number
    String,    // string
    InOut,     // IO error
    Internal,  // library specific error
    Unknown
  };

  std::string_view get_name(const JsonErrorTypes type);

  struct JsonError {
    const JsonErrorTypes type;
    const std::string message;

    JsonError() = delete;
    constexpr JsonError(const JsonError&) = default;
    constexpr JsonError(JsonError&&) = default;
    constexpr JsonError(const JsonErrorTypes t = JsonErrorTypes::Unknown)
        : type(t), message() {}
    constexpr JsonError(const JsonErrorTypes t, const std::string_view msg)
        : type(t), message(msg) {}
    constexpr JsonError(const JsonErrorTypes t, const std::string& msg)
        : type(t), message(msg) {}

    constexpr JsonError& operator=(const JsonError&) = default;
    constexpr JsonError& operator=(JsonError&&) = default;
  };

  template <typename Type> using ExpType = std::expected<Type, JsonError>;
  using UnexpJsonError = std::unexpected<JsonError>;

  constexpr UnexpJsonError makeJsonError(const JsonErrorTypes type) {
    return UnexpJsonError(std::in_place_t{}, type);
  }
  constexpr UnexpJsonError makeJsonError(const JsonErrorTypes type,
                                         const std::string_view message) {
    return UnexpJsonError(std::in_place_t{}, type, message);
  }
  constexpr UnexpJsonError makeJsonError(const JsonErrorTypes type,
                                         const std::string& message) {
    return UnexpJsonError(std::in_place_t{}, type, message);
  }

  // Expected Utils
  template <typename ResType>
  constexpr ExpType<ResType> flatten_expected(ExpType<ResType>&& value) {
    return value;
  }

  template <typename ResType>
  constexpr ExpType<ResType>
  flatten_expected(ExpType<ExpType<ResType>>&& value) {
    if (!value.has_value()) {
      return UnexpJsonError(value.error());
    }

    if (auto tmp = std::move(value.value()); tmp.has_value()) {
      if constexpr (std::is_void_v<ResType>) {
        return tmp;
      } else {
        return flatten_expected(tmp.value());
      }
    } else {
      return UnexpJsonError(tmp.error());
    }
  }

} // namespace JsonTypedefCodeGen
