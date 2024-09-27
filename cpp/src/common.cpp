#include "common.hpp"

using namespace std::string_view_literals;

namespace JsonTypedefCodeGen {

  std::string_view get_name(const JsonTypes type) {
    switch (type) {
    case JsonTypes::Null:
      return "Null"sv;
    case JsonTypes::Bool:
      return "Bool"sv;
    case JsonTypes::Number:
      return "Number"sv;
    case JsonTypes::Array:
      return "Array"sv;
    case JsonTypes::Object:
      return "Object"sv;
    case JsonTypes::String:
      return "String"sv;
    default:
      return "Invalid"sv;
    }
  }

  // --------------------------------------
  std::string_view get_name(const JsonErrorTypes type) {
    switch (type) {
    case JsonErrorTypes::Invalid:
      return "Invalid"sv;
    case JsonErrorTypes::WrongType:
      return "Wrong Type"sv;
    case JsonErrorTypes::Number:
      return "Number"sv;
    case JsonErrorTypes::String:
      return "String"sv;
    case JsonErrorTypes::InOut:
      return "I/O"sv;
    case JsonErrorTypes::Internal:
      return "Internal"sv;
    default:
      return "Unknown"sv;
    }
  }

} // namespace JsonTypedefCodeGen
