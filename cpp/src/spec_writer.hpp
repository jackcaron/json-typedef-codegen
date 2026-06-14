#pragma once

#include "json_writer.hpp"

#include <stack>

// internal specialization for each library
namespace JsonTypedefCodeGen::Writer::Specialization {

  class AbsSerializer : public BaseSerializer {
  public:
    virtual ~AbsSerializer() noexcept;

    [[nodiscard]] virtual ExpType<void> close() noexcept = 0;

    [[nodiscard]] virtual ExpType<void> write_null() noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_bool(const bool b) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_double(const double d) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_i64(const int64_t i) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_u64(const uint64_t u) noexcept = 0;
    [[nodiscard]] virtual ExpType<void>
    write_str(const std::string_view str) noexcept = 0;

    [[nodiscard]] virtual ExpType<void> start_object() noexcept = 0;
    [[nodiscard]] virtual ExpType<void>
    write_key(const std::string_view key) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> end_object() noexcept = 0;

    [[nodiscard]] virtual ExpType<void> start_array() noexcept = 0;
    [[nodiscard]] virtual ExpType<void> end_array() noexcept = 0;

    [[nodiscard]] ExpType<ExpType<void>>
    write_number(const Data::JsonValue& val) noexcept;
    [[nodiscard]] ExpType<ExpType<void>>
    write_val(const Data::JsonValue& val) noexcept;
    [[nodiscard]] ExpType<void> write_key_val(const std::string_view key,
                                              const Data::JsonValue& val) noexcept;

    [[nodiscard]] ExpType<void> write(const Data::JsonArray& arr) noexcept;
    [[nodiscard]] ExpType<void> write(const Data::JsonObject& obj) noexcept;
    [[nodiscard]] ExpType<void> write(const Data::JsonValue& val) noexcept;

    inline ExpVoidFn start_object_exec() {
      return [&]() {
        return start_object();
      };
    }
    inline ExpVoidFn end_object_exec() {
      return [&]() {
        return end_object();
      };
    }

    inline ExpVoidFn start_array_exec() {
      return [&]() {
        return start_array();
      };
    }
    inline ExpVoidFn end_array_exec() {
      return [&]() {
        return end_array();
      };
    }
  };

  //   -   -   -   -   -   -   -   -   -   -   -   -

  enum class States : uint8_t {
    RootArray,
    Array,

    RootObject,
    Object,
    ObjectKey
  };

  class StateBaseSerializer : public AbsSerializer {
  private:
    std::stack<States> m_states;
    std::stack<std::string> m_keys;

  protected:
    inline States state() const { return m_states.top(); }
    inline void push_state(const States state) { m_states.emplace(state); }
    inline void pop_state() { m_states.pop(); }

    inline const std::string& key() const { return m_keys.top(); }
    inline void push_key(const std::string_view key) { m_keys.emplace(key); }
    inline void pop_key() { m_keys.pop(); }

    inline bool can_close() const { return m_states.size() == 1 && m_keys.empty(); }

    [[nodiscard]] ExpType<void> can_start_object() const;
    [[nodiscard]] ExpType<void> can_end_object() const;
    [[nodiscard]] ExpType<void> can_start_array() const;
    [[nodiscard]] ExpType<void> can_end_array() const;

  public:
    StateBaseSerializer() = delete;
    StateBaseSerializer(const States init_state);
    virtual ~StateBaseSerializer();

    [[nodiscard]] virtual ExpType<void> close() noexcept = 0;

    [[nodiscard]] virtual ExpType<void> write_null() noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_bool(const bool b) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_double(const double d) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_i64(const int64_t i) noexcept = 0;
    [[nodiscard]] virtual ExpType<void> write_u64(const uint64_t u) noexcept = 0;
    [[nodiscard]] virtual ExpType<void>
    write_str(const std::string_view str) noexcept = 0;

    [[nodiscard]] virtual ExpType<void> start_object() noexcept = 0;
    [[nodiscard]] virtual ExpType<void>
    write_key(const std::string_view key) noexcept final;
    [[nodiscard]] virtual ExpType<void> end_object() noexcept = 0;

    [[nodiscard]] virtual ExpType<void> start_array() noexcept = 0;
    [[nodiscard]] virtual ExpType<void> end_array() noexcept = 0;
  };

} // namespace JsonTypedefCodeGen::Writer::Specialization
