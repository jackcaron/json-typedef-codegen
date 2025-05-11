#include "serializer.hpp"

#include "../internal.hpp"

using namespace std::string_view_literals;

inline States get_root_state(const Napi::Value& root) {
  return root.IsArray() ? States::RootArray : States::RootObject;
}

NapiSerializer::NapiSerializer(Napi::Value& root) //
    : m_root(root), m_states({get_root_state(root)}) {}

ExpType<void> NapiSerializer::close() {
  if (m_states.size() == 1 && m_jsons.empty() && m_keys.empty()) {
    return ExpType<void>();
  }
  return make_json_error(
      JsonErrorTypes::Invalid,
      "Serializer still have pending operations to complete"sv);
}

ExpType<void> NapiSerializer::end_item() {
  auto last_js = std::move(json());
  pop_json();

  switch (state()) {
  case States::ObjectKey: {
    auto last_key = std::move(m_keys.top());
    m_keys.pop();
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

ExpType<void> NapiSerializer::write_null() {
  push_json(m_root.Env().Null());
  return end_item();
}

ExpType<void> NapiSerializer::write_bool(const bool b) {
  push_json(Napi::Boolean::New(m_root.Env(), b));
  return end_item();
}

ExpType<void> NapiSerializer::write_double(const double d) {
  push_json(Napi::Number::New(m_root.Env(), d));
  return end_item();
}

ExpType<void> NapiSerializer::write_i64(const int64_t i) {
  push_json(Napi::BigInt::New(m_root.Env(), i));
  return end_item();
}

ExpType<void> NapiSerializer::write_u64(const uint64_t u) {
  push_json(Napi::BigInt::New(m_root.Env(), u));
  return end_item();
}

ExpType<void> NapiSerializer::write_str(const std::string_view str) {
  push_json(Napi::String::New(m_root.Env(), str.data(), str.size()));
  return end_item();
}

ExpType<void> NapiSerializer::start_object() {
  switch (state()) {
  case States::RootObject:
  case States::Object:
    return make_json_error(
        JsonErrorTypes::Invalid,
        "a key is required to create an object inside base object"sv);

  default:
    push_state(States::Object);
    push_json(Napi::Object::New(m_root.Env()));
    return ExpType<void>();
  }
}

ExpType<void> NapiSerializer::write_key(const std::string_view key) {
  switch (state()) {
  case States::RootObject:
  case States::Object:
  default:
    push_state(States::ObjectKey);
    m_keys.emplace(key);
    return ExpType<void>();

  case States::ObjectKey:
    return make_json_error(JsonErrorTypes::Invalid,
                           "object already has a key"sv);

  case States::RootArray:
  case States::Array:
    return make_json_error(JsonErrorTypes::Invalid,
                           "cannot write a key in an array"sv);
  }
}

ExpType<void> NapiSerializer::end_object() {
  switch (state()) {
  case States::RootArray:
  case States::Array:
    return make_json_error(JsonErrorTypes::Invalid,
                           "cannot end an array as an object"sv);

  case States::RootObject:
    return make_json_error(JsonErrorTypes::Invalid, "cannot end root object"sv);

  case States::Object:
  default:
    pop_state(); // move out of the object
    return end_item();

  case States::ObjectKey:
    return make_json_error(JsonErrorTypes::Invalid,
                           "closing an object before resolving a key"sv);
  }
}

ExpType<void> NapiSerializer::start_array() {
  switch (state()) {
  case States::RootObject:
  case States::Object:
    return make_json_error(
        JsonErrorTypes::Invalid,
        "a key is required to create an array inside an object"sv);

  default:
    push_state(States::Array);
    push_json(Napi::Array::New(m_root.Env()));
    return ExpType<void>();
  }
}

ExpType<void> NapiSerializer::end_array() {
  switch (state()) {
  case States::RootArray:
    return make_json_error(JsonErrorTypes::Invalid, "cannot end root array"sv);

  case States::Array:
    pop_state(); // move out of the array
    return end_item();

  default:
    return make_json_error(JsonErrorTypes::Invalid,
                           "cannot end an object as an array"sv);
  }
}

Serializer NapiSerializer::create(Napi::Value& root) {
  return create_serializer(std::make_unique<NapiSerializer>(root));
}

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
