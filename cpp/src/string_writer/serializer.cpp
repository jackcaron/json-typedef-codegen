#include "serializer.hpp"

#include "../../include/string_serializer.hpp"
#include "../internal.hpp"

#include <format>
#include <memory>

using namespace std::string_view_literals;
using namespace JsonTypedefCodeGen::Writer::Specialization;

// -------------------------------------------
InternalStringSerializer::~InternalStringSerializer() {}

ExpType<void> InternalStringSerializer::close() { return m_str_ser->close(); }

ExpType<void> InternalStringSerializer::write_null() {
  return m_str_ser->write_null();
}
ExpType<void> InternalStringSerializer::write_bool(const bool b) {
  return m_str_ser->write_bool(b);
}
ExpType<void> InternalStringSerializer::write_double(const double d) {
  return m_str_ser->write_double(d);
}
ExpType<void> InternalStringSerializer::write_i64(const int64_t i) {
  return m_str_ser->write_i64(i);
}
ExpType<void> InternalStringSerializer::write_u64(const uint64_t u) {
  return m_str_ser->write_u64(u);
}
ExpType<void> InternalStringSerializer::write_str(const std::string_view str) {
  return m_str_ser->write_str(str);
}

ExpType<void> InternalStringSerializer::start_object() {
  return m_str_ser->start_object();
}
ExpType<void> InternalStringSerializer::write_key(const std::string_view key) {
  return m_str_ser->write_key(key);
}
ExpType<void> InternalStringSerializer::end_object() {
  return m_str_ser->end_object();
}

ExpType<void> InternalStringSerializer::start_array() {
  return m_str_ser->start_array();
}
ExpType<void> InternalStringSerializer::end_array() {
  return m_str_ser->end_array();
}

Serializer InternalStringSerializer::create(StringSerializer& str_ser) {
  return create_serializer(std::make_unique<InternalStringSerializer>(str_ser));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Writer {

  StringSerializer::StringSerializer(const StringSerializerCreateInfo& info)
      : m_pretty(info.pretty), //
        m_indent(info.depth),  //
        m_indent_str(info.indent) {
    m_status.emplace(Status{.is_array = info.start_as_array,
                            .is_first_item = true,
                            .last_item_is_a_key = false});
    m_ss << (info.start_as_array ? "["sv : "{"sv);
  }

  void StringSerializer::write_indent() {
    if (m_pretty) {
      m_ss << "\n"sv;
      for (int i = 0; i < m_indent; ++i) {
        m_ss << m_indent_str;
      }
    }
  }

  void StringSerializer::end_item() {
    if (top().is_first_item) {
      top().is_first_item = false;
    } else {
      m_ss << ","sv;
      write_indent();
    }
  }

  DLL_PUBLIC ExpType<std::string> StringSerializer::to_string() const {
    if (m_closed) {
      return m_ss.str();
    }
    return make_json_error(JsonErrorTypes::Invalid,
                           "string serializer not closed"sv);
  }

  DLL_PUBLIC ExpType<void> StringSerializer::close() {
    if (m_closed) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "string serializer already closed"sv);
    }
    m_closed = true;
    --m_indent;
    write_indent();
    m_ss << (top().is_array ? "]"sv : "}"sv);
    return ExpType<void>();
  }

#define CHECK_CLOSED                                                           \
  if (m_closed) {                                                              \
    return make_json_error(JsonErrorTypes::Invalid,                            \
                           "string serializer already closed"sv);              \
  }
#define CHECK_KEY                                                              \
  if (!top().is_array) {                                                       \
    if (!top().last_item_is_a_key) {                                           \
      return make_json_error(                                                  \
          JsonErrorTypes::Invalid,                                             \
          "cannot write a value in an object without a key"sv);                \
    } else {                                                                   \
      top().last_item_is_a_key = false;                                        \
    }                                                                          \
  } else {                                                                     \
    end_item();                                                                \
  }

  DLL_PUBLIC ExpType<void> StringSerializer::write_null() {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << "null"sv;
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::write_bool(const bool b) {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << (b ? "true"sv : "false"sv);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::write_double(const double d) {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << std::format("{}", d);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::write_i64(const int64_t i) {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << std::format("{}", i);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::write_u64(const uint64_t u) {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << std::format("{}", u);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void>
  StringSerializer::write_str(const std::string_view str) {
    CHECK_CLOSED
    CHECK_KEY
    m_ss << std::format("\"{}\"", str);
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> StringSerializer::start_object() {
    CHECK_CLOSED
    CHECK_KEY

    m_ss << "{"sv;
    ++m_indent;
    write_indent();
    m_status.emplace(Status{
        .is_array = false, .is_first_item = true, .last_item_is_a_key = false});

    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void>
  StringSerializer::write_key(const std::string_view key) {
    CHECK_CLOSED

    if (top().is_array) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot write a key in an array"sv);
    }
    if (top().last_item_is_a_key) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot write two keys in a row"sv);
    }

    end_item();
    top().last_item_is_a_key = true;
    m_ss << std::format("\"{}\": ", key);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::end_object() {
    CHECK_CLOSED
    if (top().is_array) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an array as an object"sv);
    } else if (top().last_item_is_a_key) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an object with an empty key"sv);
    }
    --m_indent;
    write_indent();
    m_ss << "}";
    m_status.pop();
    if (m_status.empty()) {
      return make_json_error(JsonErrorTypes::Invalid, "empty root item"sv);
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> StringSerializer::start_array() {
    CHECK_CLOSED
    CHECK_KEY

    m_ss << "["sv;
    ++m_indent;
    write_indent();
    m_status.emplace(Status{
        .is_array = true, .is_first_item = true, .last_item_is_a_key = false});

    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StringSerializer::end_array() {
    CHECK_CLOSED
    if (!top().is_array) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an object as an array"sv);
    }
    --m_indent;
    write_indent();
    m_ss << "]";
    m_status.pop();
    if (m_status.empty()) {
      return make_json_error(JsonErrorTypes::Invalid, "empty root item"sv);
    }
    return ExpType<void>();
  }

#undef CHECK_KEY
#undef CHECK_CLOSED

  DLL_PUBLIC StringSerializer
  StringSerializer::create(const StringSerializerCreateInfo& info) {
    StringSerializerCreateInfo copy = info;
    if (copy.pretty && copy.indent.empty()) {
      copy.indent = "  "sv;
    }
    if (copy.depth < 1) {
      copy.depth = 1;
    }
    return StringSerializer(copy);
  }

  DLL_PUBLIC ExpType<Serializer>
  to_string_serializer(StringSerializer& str_serial) {
    return InternalStringSerializer::create(str_serial);
  }

} // namespace JsonTypedefCodeGen::Writer
