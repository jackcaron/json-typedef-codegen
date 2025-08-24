#pragma once

#include "json_writer.hpp"

#include <sstream>
#include <stack>

namespace JsonTypedefCodeGen::Writer {

  struct StringSerializerCreateInfo {
    bool pretty = false;
    bool start_as_array = false;
    int depth = 1;
    std::string_view indent; // empty, it's set to "  "
  };

  class StringSerializer {
  private:
    struct Status {
      bool is_array;
      bool is_first_item;
      bool last_item_is_a_key;
    };

    std::stringstream m_ss;
    std::stack<Status> m_status;

    int m_indent = 1;
    std::string m_indent_str;
    bool m_closed = false;
    bool m_pretty = false;

    inline Status& top() { return m_status.top(); }
    bool is_first_item();
    void write_indent();
    void end_item();

    StringSerializer(const StringSerializerCreateInfo& info);

  public:
    StringSerializer() = delete;

    // must "close" the string serializer to access the final result
    ExpType<std::string> to_string() const;

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

    static StringSerializer create(const StringSerializerCreateInfo& info);
  };

  /**
   * The StringSerializer must exists longer than the Serializer
   */
  ExpType<Serializer> to_string_serializer(StringSerializer& str_serial);

} // namespace JsonTypedefCodeGen::Writer
