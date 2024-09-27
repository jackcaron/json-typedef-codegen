#include "json_reader.hpp"

using namespace JsonTypedefCodeGen;

void temporary(const Reader::JsonArray& array) {
  for (const auto value : array) {
    // ??
  }
}

int main(int argc, char** argv) {
  // later
  static_assert(std::input_iterator<Reader::JsonArrayIterator>);
  static_assert(std::input_iterator<Reader::JsonObjectIterator>);

  return 0;
}
