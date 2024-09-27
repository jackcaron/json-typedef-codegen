#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace JsonTypedefCodeGen {

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
    JsonError(const JsonErrorTypes t) : type(t), message() {}
    JsonError(const JsonErrorTypes t, const std::string_view msg)
        : type(t), message(msg) {}
    JsonError(const JsonErrorTypes t, const std::string& msg)
        : type(t), message(msg) {}
  };

  template <typename Type> using ExpType = std::expected<Type, JsonError>;
  using UnexpJsonError = std::unexpected<JsonError>;

  inline UnexpJsonError makeJsonError(const JsonErrorTypes type) {
    return UnexpJsonError(std::in_place_t{}, type);
  }
  inline UnexpJsonError makeJsonError(const JsonErrorTypes type,
                                      const std::string_view message) {
    return UnexpJsonError(std::in_place_t{}, type, message);
  }
  inline UnexpJsonError makeJsonError(const JsonErrorTypes type,
                                      const std::string& message) {
    return UnexpJsonError(std::in_place_t{}, type, message);
  }

} // namespace JsonTypedefCodeGen
