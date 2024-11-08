#include "json_reader.hpp"

#include <ranges>

using namespace JsonTypedefCodeGen;

struct NotEmpty {
  inline auto operator()(const auto& ex) -> bool { return ex.has_value(); }
};

void temporary(const Reader::JsonArray& array) {
  for (const auto& value : array | std::views::filter(NotEmpty{})) {
    // ??
  }
}

void temporary2(const Reader::JsonObject& obj) {
  for (const auto& it : obj | std::views::filter(NotEmpty{})) {
    // ??
  }
}

void temporary3(const Reader::JsonValue& val) {
  for (const auto& value : val.read_array().value_or(Reader::JsonArray()) |
                               std::views::filter(NotEmpty{})) {
    // ??
  }
}

int main(int argc, char** argv) {
  // later
  static_assert(std::input_iterator<Reader::JsonArrayIterator>);
  static_assert(std::input_iterator<Reader::JsonObjectIterator>);

  static_assert(std::ranges::range<Reader::JsonArray>);
  static_assert(std::ranges::range<Reader::JsonObject>);

  static_assert(std::ranges::viewable_range<Reader::JsonArray>);
  static_assert(std::ranges::viewable_range<Reader::JsonObject>);

  return 0;
}
