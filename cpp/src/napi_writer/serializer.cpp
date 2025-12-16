
#include "../internal.hpp"
#include "../spec_writer.hpp"

#include <napi.h>
using namespace std::string_view_literals;

using namespace JsonTypedefCodeGen;
using namespace JsonTypedefCodeGen::Writer;
using namespace JsonTypedefCodeGen::Writer::Specialization;

namespace {

  inline States get_root_state(const Napi::Value& root) {
    return root.IsArray() ? States::RootArray : States::RootObject;
  }

  class NapiSerializer final : public Specialization::StateBaseSerializer {
  private:
    Napi::Value m_root;
    std::stack<Napi::Value> m_jsons;

    inline Napi::Value& json() { return m_jsons.top(); }
    inline void push_json(Napi::Value js) { m_jsons.emplace(js); }
    inline void pop_json() { m_jsons.pop(); }

    ExpType<void> end_item() {
      auto last_js = std::move(json());
      pop_json();

      switch (state()) {
      case States::ObjectKey: {
        auto last_key = std::move(key());
        pop_key();
        pop_state(); // go back to an object state

        switch (state()) {
        case States::RootObject:
          m_root.As<Napi::Object>().Set(last_key, last_js);
          break;
        case States::Object:
          json().As<Napi::Object>().Set(last_key, last_js);
          break;
        default:
          return make_json_error(JsonErrorTypes::Invalid,
                                 "expected to be an object"sv);
        }
      } break;

      case States::RootArray: {
        auto sz = m_root.As<Napi::Array>().Length();
        m_root.As<Napi::Array>().Set(sz, last_js);
      } break;

      case States::Array: {
        auto sz = json().As<Napi::Array>().Length();
        json().As<Napi::Array>().Set(sz, last_js);
      } break;

      default:
        return make_json_error(JsonErrorTypes::Invalid,
                               "adding an item in an object without a key"sv);
      }
      return ExpType<void>();
    }

  public:
    NapiSerializer() = delete;
    NapiSerializer(Napi::Value& root)
        : StateBaseSerializer(get_root_state(root)), m_root(root) {}
    ~NapiSerializer() {}

    virtual ExpType<void> close() override {
      if (can_close() && m_jsons.empty()) {
        return ExpType<void>();
      }
      return make_json_error(
          JsonErrorTypes::Invalid,
          "Serializer still have pending operations to complete"sv);
    }

    virtual ExpType<void> write_null() override {
      push_json(m_root.Env().Null());
      return end_item();
    }
    virtual ExpType<void> write_bool(const bool b) override {
      push_json(Napi::Boolean::New(m_root.Env(), b));
      return end_item();
    }
    virtual ExpType<void> write_double(const double d) override {
      push_json(Napi::Number::New(m_root.Env(), d));
      return end_item();
    }
    virtual ExpType<void> write_i64(const int64_t i) override {
      push_json(Napi::BigInt::New(m_root.Env(), i));
      return end_item();
    }
    virtual ExpType<void> write_u64(const uint64_t u) override {
      push_json(Napi::BigInt::New(m_root.Env(), u));
      return end_item();
    }
    virtual ExpType<void> write_str(const std::string_view str) override {
      push_json(Napi::String::New(m_root.Env(), str.data(), str.size()));
      return end_item();
    }

    virtual ExpType<void> start_object() override {
      return can_start_object().transform([&]() -> void {
        push_state(States::Object);
        push_json(Napi::Object::New(m_root.Env()));
      });
    }
    virtual ExpType<void> end_object() override {
      return flatten_expected(
          can_end_object().transform([&]() -> ExpType<void> {
            pop_state();
            return end_item();
          }));
    }

    virtual ExpType<void> start_array() override {
      return can_start_array().transform([&]() -> void {
        push_state(States::Array);
        push_json(Napi::Array::New(m_root.Env()));
      });
    }
    virtual ExpType<void> end_array() override {
      return flatten_expected(can_end_array().transform([&]() -> ExpType<void> {
        pop_state(); // move out of the array
        return end_item();
      }));
    }

    static Serializer create(Napi::Value& root) {
      return create_serializer(std::make_unique<NapiSerializer>(root));
    }
  };

} // namespace

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Writer {

  DLL_PUBLIC ExpType<Serializer> napi_serializer(Napi::Value& root) {
    switch (root.Type()) {
    case napi_object:
      return NapiSerializer::create(root);

    default:
      return make_json_error(
          JsonErrorTypes::Invalid,
          "can only create a serializer on an object or an array"sv);
    }
  }

} // namespace JsonTypedefCodeGen::Writer
