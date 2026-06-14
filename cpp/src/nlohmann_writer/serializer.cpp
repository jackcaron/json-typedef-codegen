
#include "../internal.hpp"

#include "../spec_writer.hpp"
#include "nlohmann/json.hpp"

#include <stack>

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;
using namespace JsonTypedefCodeGen::Writer::Specialization;
using namespace std::string_view_literals;

using NType = nlohmann::detail::value_t;

namespace {

  using NJson = nlohmann::json;

  inline States get_root_state(const NJson& root) {
    return root.type() == NType::object ? States::RootObject : States::RootArray;
  }

  inline auto create_nloh_error(const std::string_view message) -> auto {
    return make_json_error(JsonErrorTypes::Internal, message);
  }

  class NlohSerializer final : public Specialization::StateBaseSerializer {
  private:
    NJson& m_root;
    std::stack<NJson> m_jsons;

    inline NJson& json() { return m_jsons.top(); }
    inline void push_json(NJson js) { m_jsons.emplace(js); }
    inline void pop_json() { m_jsons.pop(); }

    ExpType<void> catch_push(NJson& obj, NJson& last_js) noexcept {
      try {
        obj.push_back(std::move(last_js));
      } catch (const std::bad_alloc& ba) {
        return create_nloh_error(ba.what());
      } catch (const nlohmann::detail::type_error& e) {
        return create_nloh_error(e.what());
      }
      return ExpType<void>();
    }

    ExpType<void> end_item() noexcept {
      auto last_js = std::move(json());
      pop_json();

      switch (state()) {
      case States::ObjectKey: {
        auto last_key = std::move(key());
        pop_key();
        pop_state(); // go back to an object state

        switch (state()) {
        case States::RootObject:
          m_root[last_key] = std::move(last_js);
          break;
        case States::Object:
          json()[last_key] = std::move(last_js);
          break;
        default:
          return make_json_error(JsonErrorTypes::Invalid,
                                 "expected to be an object"sv);
        }
      } break;

      case States::RootArray:
        return catch_push(m_root, last_js);

      case States::Array:
        return catch_push(json(), last_js);

      default:
        return make_json_error(JsonErrorTypes::Invalid,
                               "adding an item in an object without a key"sv);
      }
      return ExpType<void>();
    }

  public:
    NlohSerializer() = delete;
    NlohSerializer(NJson& root)
        : StateBaseSerializer(get_root_state(root)), m_root(root) {}
    ~NlohSerializer() {}

    virtual ExpType<void> close() override {
      if (can_close() && m_jsons.empty()) {
        return ExpType<void>();
      }
      return make_json_error(
          JsonErrorTypes::Invalid,
          "Serializer still have pending operations to complete"sv);
    }

    virtual ExpType<void> write_null() override {
      push_json(NJson(nullptr));
      return end_item();
    }
    virtual ExpType<void> write_bool(const bool b) override {
      push_json(NJson(b));
      return end_item();
    }
    virtual ExpType<void> write_double(const double d) override {
      push_json(NJson(d));
      return end_item();
    }
    virtual ExpType<void> write_i64(const int64_t i) override {
      push_json(NJson(i));
      return end_item();
    }
    virtual ExpType<void> write_u64(const uint64_t u) override {
      push_json(NJson(u));
      return end_item();
    }
    virtual ExpType<void> write_str(const std::string_view str) override {
      push_json(NJson(str));
      return end_item();
    }

    virtual ExpType<void> start_object() override {
      return can_start_object().transform([&]() -> void {
        push_state(States::Object);
        push_json(NJson::object());
      });
    }
    virtual ExpType<void> end_object() override {
      return flatten_expected(can_end_object().transform([&]() -> ExpType<void> {
        pop_state();
        return end_item();
      }));
    }

    virtual ExpType<void> start_array() override {
      return can_start_array().transform([&]() -> void {
        push_state(States::Array);
        push_json(NJson::array());
      });
    }
    virtual ExpType<void> end_array() override {
      return flatten_expected(can_end_array().transform([&]() -> ExpType<void> {
        pop_state(); // move out of the array
        return end_item();
      }));
    }

    static Serializer create(NJson& root) {
      return create_serializer(std::make_unique<NlohSerializer>(root));
    }
  };

} // namespace

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Writer {

  DLL_PUBLIC ExpType<Serializer> nlohmann_serializer(NJson& root) {
    switch (root.type()) {
    case NType::array:
    case NType::object:
      return NlohSerializer::create(root);

    default:
      return make_json_error(
          JsonErrorTypes::Invalid,
          "can only create a serializer on an object or an array"sv);
    }
  }

} // namespace JsonTypedefCodeGen::Writer
