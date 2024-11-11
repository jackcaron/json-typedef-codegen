#pragma once

#include <expected>

#ifdef _DEBUG
#define DLL_PUBLIC
#else
#define DLL_PUBLIC __attribute__((visibility("default")))
#endif

// UNTIL std::expected has "transform" implemented
template <typename Dst, typename Src, typename Err, class F>
requires requires(Src s, F f) { requires std::same_as<decltype(f(s)), Dst>; }
auto transform_expected(std::expected<Src, Err> value, F&& f)
    -> std::expected<Dst, Err> {
  if (value.has_value()) {
    return f(value.value());
  }
  return std::unexpected<Err>(std::move(value.error()));
}

//
