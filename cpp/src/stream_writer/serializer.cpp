#include "serializer.hpp"

#include "../../include/stream_serializer.hpp"
#include "../internal.hpp"

#include <format>
#include <memory>

using namespace std::string_view_literals;
using namespace JsonTypedefCodeGen::Writer::Specialization;

// -------------------------------------------
InternalStreamSerializer::~InternalStreamSerializer() {}

ExpType<void> InternalStreamSerializer::close() { return m_str_ser->close(); }

ExpType<void> InternalStreamSerializer::write_null() {
  return m_str_ser->write_null();
}
ExpType<void> InternalStreamSerializer::write_bool(const bool b) {
  return m_str_ser->write_bool(b);
}
ExpType<void> InternalStreamSerializer::write_double(const double d) {
  return m_str_ser->write_double(d);
}
ExpType<void> InternalStreamSerializer::write_i64(const int64_t i) {
  return m_str_ser->write_i64(i);
}
ExpType<void> InternalStreamSerializer::write_u64(const uint64_t u) {
  return m_str_ser->write_u64(u);
}
ExpType<void> InternalStreamSerializer::write_str(const std::string_view str) {
  return m_str_ser->write_str(str);
}

ExpType<void> InternalStreamSerializer::start_object() {
  return m_str_ser->start_object();
}
ExpType<void> InternalStreamSerializer::write_key(const std::string_view key) {
  return m_str_ser->write_key(key);
}
ExpType<void> InternalStreamSerializer::end_object() {
  return m_str_ser->end_object();
}

ExpType<void> InternalStreamSerializer::start_array() {
  return m_str_ser->start_array();
}
ExpType<void> InternalStreamSerializer::end_array() {
  return m_str_ser->end_array();
}

Serializer InternalStreamSerializer::create(StreamSerializer& str_ser) {
  return create_serializer(std::make_unique<InternalStreamSerializer>(str_ser));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Writer {

  StreamSerializer::StreamSerializer(const StreamSerializerCreateInfo& info)
      : m_os(info.output_stream),               //
        m_pretty(info.pretty),                  //
        m_close_root_item(info.open_root_item), //
        m_indent(info.depth),                   //
        m_indent_str(info.indent) {
    m_status.emplace(Status{.is_array = info.start_as_array,
                            .is_first_item = true,
                            .last_item_is_a_key = false});
    if (info.open_root_item) {
      (*m_os) << (info.start_as_array ? "["sv : "{"sv);
    }
  }

  void StreamSerializer::write_indent() {
    if (m_pretty) {
      (*m_os) << "\n"sv;
      for (int i = 0; i < m_indent; ++i) {
        (*m_os) << m_indent_str;
      }
    }
  }

  void StreamSerializer::end_item() {
    if (top().is_first_item) {
      top().is_first_item = false;
    } else {
      (*m_os) << ","sv;
      write_indent();
    }
  }

  DLL_PUBLIC ExpType<void> StreamSerializer::close() {
    if (m_closed) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "string serializer already closed"sv);
    }
    m_closed = true;
    --m_indent;
    if (m_close_root_item) {
      write_indent();
      (*m_os) << (top().is_array ? "]"sv : "}"sv);
    }
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

  DLL_PUBLIC ExpType<void> StreamSerializer::write_null() {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << "null"sv;
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::write_bool(const bool b) {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << (b ? "true"sv : "false"sv);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::write_double(const double d) {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << std::format("{}"sv, d);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::write_i64(const int64_t i) {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << std::format("{}"sv, i);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::write_u64(const uint64_t u) {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << std::format("{}"sv, u);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void>
  StreamSerializer::write_str(const std::string_view str) {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << std::format("\"{}\""sv, str);
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> StreamSerializer::start_object() {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << "{"sv;
    ++m_indent;
    write_indent();
    m_status.emplace(Status{
        .is_array = false, .is_first_item = true, .last_item_is_a_key = false});

    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void>
  StreamSerializer::write_key(const std::string_view key) {
    CHECK_CLOSED;

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

    (*m_os) << std::format("\"{}\": "sv, key);
    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::end_object() {
    CHECK_CLOSED;
    if (top().is_array) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an array as an object"sv);
    } else if (top().last_item_is_a_key) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an object with an empty key"sv);
    }
    --m_indent;
    write_indent();
    (*m_os) << "}"sv;
    m_status.pop();
    if (m_status.empty()) {
      return make_json_error(JsonErrorTypes::Invalid, "empty root item"sv);
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> StreamSerializer::start_array() {
    CHECK_CLOSED;
    CHECK_KEY;

    (*m_os) << "["sv;
    ++m_indent;
    write_indent();
    m_status.emplace(Status{
        .is_array = true, .is_first_item = true, .last_item_is_a_key = false});

    return ExpType<void>();
  }
  DLL_PUBLIC ExpType<void> StreamSerializer::end_array() {
    CHECK_CLOSED;
    if (!top().is_array) {
      return make_json_error(JsonErrorTypes::Invalid,
                             "cannot end an object as an array"sv);
    }
    --m_indent;
    write_indent();
    (*m_os) << "]"sv;
    m_status.pop();
    if (m_status.empty()) {
      return make_json_error(JsonErrorTypes::Invalid, "empty root item"sv);
    }
    return ExpType<void>();
  }

#undef CHECK_KEY
#undef CHECK_CLOSED

  DLL_PUBLIC ExpType<StreamSerializer>
  StreamSerializer::create(const StreamSerializerCreateInfo& info) {
    if (info.output_stream == nullptr) {
      return make_json_error(
          JsonErrorTypes::InOut,
          "missing output stream in StreamSerializerCreateInfo"sv);
    }

    StreamSerializerCreateInfo copy = info;
    if (copy.pretty && copy.indent.empty()) {
      copy.indent = "  "sv;
    }
    if (copy.depth < 1) {
      copy.depth = 1;
    }
    return StreamSerializer(copy);
  }

  DLL_PUBLIC ExpType<Serializer>
  to_stream_serializer(StreamSerializer& str_serial) {
    return InternalStreamSerializer::create(str_serial);
  }

} // namespace JsonTypedefCodeGen::Writer
