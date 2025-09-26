#pragma once

#include "json_writer.hpp"

#include <sstream>
#include <stack>

namespace JsonTypedefCodeGen::Writer {

  struct StreamSerializerCreateInfo {
    std::ostream* output_stream = nullptr;

    bool pretty = false;
    bool start_as_array = false;
    bool open_root_item = false;
    int depth = 1;
    std::string_view indent; // empty, it's set to "  "
  };

  class StreamSerializer {
  private:
    struct Status {
      bool is_array;
      bool is_first_item;
      bool last_item_is_a_key;
    };

    std::ostream* m_os = nullptr;
    std::stack<Status> m_status;

    int m_indent = 1;
    std::string m_indent_str;
    bool m_closed = false;
    bool m_pretty = false;
    bool m_close_root_item = false;

    inline Status& top() { return m_status.top(); }
    void write_indent();
    void end_item();

    StreamSerializer(const StreamSerializerCreateInfo& info);

  public:
    StreamSerializer() = delete;

    // to close the root item
    ExpType<void> close();

    ExpType<void> write_null();
    ExpType<void> write_bool(const bool b);
    ExpType<void> write_double(const double d);
    ExpType<void> write_i64(const int64_t i);
    ExpType<void> write_u64(const uint64_t u);
    ExpType<void> write_str(const std::string_view str);

    ExpType<void> start_object();
    ExpType<void> write_key(const std::string_view key);
    ExpType<void> end_object();

    ExpType<void> start_array();
    ExpType<void> end_array();

    static ExpType<StreamSerializer>
    create(const StreamSerializerCreateInfo& info);
  };

  /**
   * The StreamSerializer must exists longer than the Serializer
   */
  ExpType<Serializer> to_stream_serializer(StreamSerializer& str_serial);

} // namespace JsonTypedefCodeGen::Writer
