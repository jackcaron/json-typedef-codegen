#include "array.hpp"

#include "value.hpp"

using namespace std::string_view_literals;

ExpType<JsonValue> NapiArrayIterator::get() const {
  if (const auto val = m_array.Get(m_index); val.IsEmpty()) {
    return makeJsonError(JsonErrorTypes::Invalid, "Empty value"sv);
  } else {
    return NapiValue::create(val);
  }
}

void NapiArrayIterator::next() {
  if (!m_array.IsEmpty() && m_index < m_array.Length()) {
    ++m_index;
  }
}

bool NapiArrayIterator::done() const {
  return m_array.IsEmpty() || m_index == m_array.Length();
}

JsonArrayIterator NapiArrayIterator::create(const Napi::Array arr) {
  return create_json(std::move(std::make_unique<NapiArrayIterator>(arr)));
}

// -------------------------------------------
JsonArrayIterator NapiArray::begin() const {
  return NapiArrayIterator::create(m_array);
}

JsonArray NapiArray::create(const Napi::Array arr) {
  return create_json(std::move(std::make_unique<NapiArray>(arr)));
}
