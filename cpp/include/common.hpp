#pragma once

#include "headers.hpp"

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

  using UnexpJsonError = std::unexpected<JsonError>;
  template <typename Type> using ExpType = std::expected<Type, JsonError>;

  constexpr UnexpJsonError make_json_error(const JsonErrorTypes type) {
    return UnexpJsonError(std::in_place_t{}, type);
  }
  constexpr UnexpJsonError make_json_error(const JsonErrorTypes type,
                                           const std::string_view message) {
    return UnexpJsonError(std::in_place_t{}, type, message);
  }

  // Portable try-like macros for ExpType<T> (avoid GCC extensions)
  // Usage:
  //   TRY_ASSIGN(val, some_function()); // declares `val` with the inner value
  //   TRY_VOID(some_void_function());    // returns early on error for void flows

#define PP_CONCAT_IMPL(a, b) a##b
#define PP_CONCAT(a, b) PP_CONCAT_IMPL(a, b)

#define TRY_ASSIGN(var, expr)                                                       \
  auto PP_CONCAT(_expected_tmp_, __LINE__) = (expr);                                \
  if (!PP_CONCAT(_expected_tmp_, __LINE__).has_value()) [[unlikely]] {              \
    return JsonTypedefCodeGen::UnexpJsonError(                                      \
        PP_CONCAT(_expected_tmp_, __LINE__).error());                               \
  }                                                                                 \
  var = std::move(PP_CONCAT(_expected_tmp_, __LINE__)).value();

#define TRY_VOID(expr)                                                              \
  if (auto _expected_tmp_ = (expr); !_expected_tmp_.has_value()) [[unlikely]] {     \
    return JsonTypedefCodeGen::UnexpJsonError(std::move(_expected_tmp_).error());   \
  }

  template <typename Type> using JsonMap = std::map<std::string, Type>;

  // Expected Utils
  template <typename ResType>
  [[nodiscard]] constexpr ExpType<ResType> flatten_expected(ResType&& value) {
    return ExpType<ResType>(std::move(value));
  }

  template <typename ResType>
  [[nodiscard]] constexpr ExpType<ResType>
  flatten_expected(ExpType<ResType>&& value) {
    return value;
  }

  template <typename ResType>
  [[nodiscard]] constexpr ExpType<ResType>
  flatten_expected(ExpType<ExpType<ResType>>&& value) {
    if (!value.has_value()) [[unlikely]] {
      return UnexpJsonError(value.error());
    }

    if (auto tmp = std::move(value.value()); tmp.has_value()) {
      if constexpr (std::is_void_v<ResType>) {
        return tmp;
      } else {
        return flatten_expected(std::move(tmp.value()));
      }
    } else {
      return UnexpJsonError(tmp.error());
    }
  }

  [[nodiscard]] constexpr ExpType<void> chain_void_expected(ExpType<void> last) {
    return last;
  }

  template <typename... Xp>
  [[nodiscard]] constexpr ExpType<void> chain_void_expected(ExpType<void> first,
                                                            Xp... etc) {
    if (first.has_value()) [[likely]] {
      return chain_void_expected(etc...);
    }
    return first;
  }

  [[nodiscard]] constexpr ExpType<void>
  chain_void_expected(std::initializer_list<ExpType<void>> list) {
    for (auto& item : list) {
      if (!item.has_value()) [[unlikely]] {
        return item;
      }
    }
    return ExpType<void>();
  }

  using ExpVoidFn = std::function<ExpType<void>()>;
  [[nodiscard]] constexpr ExpType<void> chain_exec_void_expected(ExpVoidFn fn) {
    return fn();
  }

  template <typename... Xp>
  [[nodiscard]] constexpr ExpType<void> chain_exec_void_expected(ExpVoidFn first,
                                                                 Xp... etc) {
    TRY_VOID(first());
    return chain_exec_void_expected(etc...);
  }

} // namespace JsonTypedefCodeGen
