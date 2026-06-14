
#include "../../include/stream_serializer.hpp"
#include "../internal.hpp"
#include "../spec_writer.hpp"

#include <format>
#include <memory>
#include <stack>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;
using namespace JsonTypedefCodeGen::Writer::Specialization;
using namespace std::string_view_literals;

// -------------------------------------------

namespace {

  class InternalStreamSerializer final : public Specialization::AbsSerializer {
  private:
    StreamSerializer* m_str_ser = nullptr;

  public:
    InternalStreamSerializer() = delete;
    InternalStreamSerializer(StreamSerializer& str_ser) : m_str_ser(&str_ser) {}
    ~InternalStreamSerializer() {}

    virtual ExpType<void> close() override { return m_str_ser->close(); }

    virtual ExpType<void> write_null() override { return m_str_ser->write_null(); }
    virtual ExpType<void> write_bool(const bool b) override {
      return m_str_ser->write_bool(b);
    }
    virtual ExpType<void> write_double(const double d) override {
      return m_str_ser->write_double(d);
    }
    virtual ExpType<void> write_i64(const int64_t i) override {
      return m_str_ser->write_i64(i);
    }
    virtual ExpType<void> write_u64(const uint64_t u) override {
      return m_str_ser->write_u64(u);
    }
    virtual ExpType<void> write_str(const std::string_view str) override {
      return m_str_ser->write_str(str);
    }

    virtual ExpType<void> start_object() override {
      return m_str_ser->start_object();
    }
    virtual ExpType<void> write_key(const std::string_view key) override {
      return m_str_ser->write_key(key);
    }
    virtual ExpType<void> end_object() override { return m_str_ser->end_object(); }

    virtual ExpType<void> start_array() override { return m_str_ser->start_array(); }
    virtual ExpType<void> end_array() override { return m_str_ser->end_array(); }

    static Serializer create(StreamSerializer& str_ser) {
      return create_serializer(std::make_unique<InternalStreamSerializer>(str_ser));
    }
  };

} // namespace

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

#define CHECK_CLOSED                                                                \
  if (m_closed) {                                                                   \
    return make_json_error(JsonErrorTypes::Invalid,                                 \
                           "string serializer already closed"sv);                   \
  }
#define CHECK_KEY                                                                   \
  if (!top().is_array) {                                                            \
    if (!top().last_item_is_a_key) {                                                \
      return make_json_error(JsonErrorTypes::Invalid,                               \
                             "cannot write a value in an object without a key"sv);  \
    } else {                                                                        \
      top().last_item_is_a_key = false;                                             \
    }                                                                               \
  } else {                                                                          \
    end_item();                                                                     \
  }

  DLL_PUBLIC ExpType<void> StreamSerializer::close() {
    CHECK_CLOSED;

    m_closed = true;
    --m_indent;
    if (m_close_root_item) {
      write_indent();
      (*m_os) << (top().is_array ? "]"sv : "}"sv);
    }
    return ExpType<void>();
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
  DLL_PUBLIC ExpType<void> StreamSerializer::write_str(const std::string_view str) {
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
  DLL_PUBLIC ExpType<void> StreamSerializer::write_key(const std::string_view key) {
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

    (*m_os) << std::format("\"{}\":"sv, key);
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

  DLL_PUBLIC ExpType<Serializer> to_stream_serializer(StreamSerializer& str_serial) {
    return InternalStreamSerializer::create(str_serial);
  }

} // namespace JsonTypedefCodeGen::Writer
