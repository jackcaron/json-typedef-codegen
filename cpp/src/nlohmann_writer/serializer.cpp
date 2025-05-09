#include "serializer.hpp"

#include "../internal.hpp"

using namespace std::string_view_literals;
using NType = nlohmann::detail::value_t;

NlohSerializer::NlohSerializer(NJson root) {
  m_states.emplace(root.type() == NType::object ? States::RootObject
                                                : States::RootArray);
  m_jsons.emplace(root);
}

ExpType<void> NlohSerializer::end_item() {
  auto last_js = json();
  pop_json();

  switch (state()) {
  case States::ObjectKey: {
    auto last_key = std::move(m_keys.top());
    m_keys.pop();
    json()[last_key] = last_js;
    pop_state(); // go back to an object state
  } break;

  case States::RootArray:
  case States::Array:
    json().push_back(last_js);
    break;

  default:
    return make_json_error(JsonErrorTypes::Invalid,
                           "adding an item in an object without a key"sv);
  }
  return ExpType<void>();
}

ExpType<void> NlohSerializer::write_null() {
  push_json(NJson(nullptr));
  return end_item();
}

ExpType<void> NlohSerializer::write_bool(const bool b) {
  push_json(NJson(b));
  return end_item();
}

ExpType<void> NlohSerializer::write_double(const double d) {
  push_json(NJson(d));
  return end_item();
}

ExpType<void> NlohSerializer::write_i64(const int64_t i) {
  push_json(NJson(i));
  return end_item();
}

ExpType<void> NlohSerializer::write_u64(const uint64_t u) {
  push_json(NJson(u));
  return end_item();
}

ExpType<void> NlohSerializer::write_str(const std::string_view str) {
  push_json(NJson(str));
  return end_item();
}

ExpType<void> NlohSerializer::start_object() {
  switch (state()) {
  case States::RootObject:
  case States::Object:
    return make_json_error(
        JsonErrorTypes::Invalid,
        "a key is required to create an object inside base object"sv);

  default:
    push_state(States::Object);
    push_json(NJson::object());
    return ExpType<void>();
  }
}

ExpType<void> NlohSerializer::write_key(const std::string_view key) {
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

ExpType<void> NlohSerializer::end_object() {
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

ExpType<void> NlohSerializer::start_array() {
  switch (state()) {
  case States::RootObject:
  case States::Object:
    return make_json_error(
        JsonErrorTypes::Invalid,
        "a key is required to create an array inside an object"sv);

  default:
    push_state(States::Array);
    push_json(NJson::array());
    return ExpType<void>();
  }
}

ExpType<void> NlohSerializer::end_array() {
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

Serializer NlohSerializer::create(NJson root) {
  return create_serializer(std::make_unique<NlohSerializer>(root));
}

// -------------------------------------------
// -------------------------------------------
namespace JsonTypedefCodeGen::Writer {

  DLL_PUBLIC ExpType<Serializer> nlohmann_serializer(NJson root) {
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
