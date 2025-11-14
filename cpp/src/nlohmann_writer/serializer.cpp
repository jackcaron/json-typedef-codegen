#include "serializer.hpp"

#include "../internal.hpp"

using namespace std::string_view_literals;
using NType = nlohmann::detail::value_t;
using namespace JsonTypedefCodeGen::Writer::Specialization;

inline States get_root_state(const NJson& root) {
  return root.type() == NType::object ? States::RootObject : States::RootArray;
}

NlohSerializer::NlohSerializer(NJson& root) //
    : StateBaseSerializer(get_root_state(root)), m_root(root) {}

ExpType<void> NlohSerializer::close() {
  if (can_close() && m_jsons.empty()) {
    return ExpType<void>();
  }
  return make_json_error(
      JsonErrorTypes::Invalid,
      "Serializer still have pending operations to complete"sv);
}

ExpType<void> NlohSerializer::end_item() {
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
    m_root.push_back(std::move(last_js));
    break;

  case States::Array:
    json().push_back(std::move(last_js));
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
  return can_start_object().transform([&]() -> void {
    push_state(States::Object);
    push_json(NJson::object());
  });
}

ExpType<void> NlohSerializer::end_object() {
  return flatten_expected(can_end_object().transform([&]() -> ExpType<void> {
    pop_state();
    return end_item();
  }));
}

ExpType<void> NlohSerializer::start_array() {
  return can_start_array().transform([&]() -> void {
    push_state(States::Array);
    push_json(NJson::array());
  });
}

ExpType<void> NlohSerializer::end_array() {
  return flatten_expected(can_end_array().transform([&]() -> ExpType<void> {
    pop_state(); // move out of the array
    return end_item();
  }));
}

Serializer NlohSerializer::create(NJson& root) {
  return create_serializer(std::make_unique<NlohSerializer>(root));
}

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
