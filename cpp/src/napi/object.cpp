#include "object.hpp"

#include "value.hpp"

using namespace std::string_view_literals;

NapiObjectIterator::NapiObjectIterator(Napi::Object object) : m_object(object) {
  auto maybe = object.GetPropertyNames();
  if (maybe.IsArray()) {
    m_keys = maybe.As<Napi::Array>();
  }
}

ExpType<ObjectIteratorPair> NapiObjectIterator::get() const {
  if (m_keys.IsEmpty()) {
    return makeJsonError(JsonErrorTypes::Invalid, "Object with no keys"sv);
  }

  const auto key = m_keys.Get(m_index);
  if (key.IsEmpty()) {
    return makeJsonError(JsonErrorTypes::Invalid, "Empty key"sv);
  } else if (!key.IsString() && !key.IsSymbol()) {
    return makeJsonError(JsonErrorTypes::WrongType,
                         "Object's key is not a string"sv);
  }

  const auto val = m_object.Get(key);
  if (val.IsEmpty()) {
    return makeJsonError(JsonErrorTypes::Invalid, "Empty value"sv);
  }
  return ObjectIteratorPair(key.ToString(), NapiValue::create(val));
}

void NapiObjectIterator::next() {
  if (!m_keys.IsEmpty() && m_index < m_keys.Length()) {
    ++m_index;
  }
}

bool NapiObjectIterator::done() const {
  return m_keys.IsEmpty() || m_index == m_keys.Length();
}

JsonObjectIterator NapiObjectIterator::create(Napi::Object object) {
  return create_json(std::move(std::make_unique<NapiObjectIterator>(object)));
}

// -------------------------------------------
JsonObjectIterator NapiObject::begin() const {
  return NapiObjectIterator::create(m_object);
}

JsonObject NapiObject::create(const Napi::Object arr) {
  return create_json(std::move(std::make_unique<NapiObject>(arr)));
}
