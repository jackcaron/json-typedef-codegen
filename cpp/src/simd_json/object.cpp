#include "object.hpp"

#include "value.hpp"

ExpType<ObjectIteratorPair> SimdObjectIterator::get() const {
  auto err_type = m_iter.error();
  if (err_type == simdjson::SUCCESS) {
    auto unsafe_val = m_iter.value_unsafe();
    auto val = *unsafe_val;
    err_type = val.error();
    if (err_type == simdjson::SUCCESS) {
      auto field = val.value_unsafe();
      return ObjectIteratorPair{field.escaped_key(),
                                SimdValue::create(field.value())};
    }
  }
  return makeJsonError(err_type);
}

void SimdObjectIterator::next() {
  if (m_iter.error() == simdjson::SUCCESS) {
    if (m_iter.value_unsafe() == m_end) {
      // if it reached the end, invalidate it
      m_iter = ObjIterResult(simdjson::OUT_OF_BOUNDS);
    } else {
      ++(m_iter.value_unsafe());
    }
  }
}

bool SimdObjectIterator::done() const {
  return m_iter.error() != simdjson::SUCCESS || m_iter.value_unsafe() == m_end;
}

JsonObjectIterator SimdObjectIterator::create(ObjIterResult begin,
                                              ObjIter end) {
  return create_json(
      std::move(std::make_unique<SimdObjectIterator>(begin, end)));
}

// -------------------------------------------
JsonObjectIterator SimdObject::begin() const {
  auto first = m_object.begin(), last = m_object.end();
  ObjIter end; // default and invalid, never used if there's an error

  if (const auto err_type = last.error(); err_type != simdjson::SUCCESS) {
    first = ObjIterResult(err_type); // forward the error on the first iterator
  } else {
    end = last.value_unsafe();
  }
  return SimdObjectIterator::create(first, end);
}

JsonObject SimdObject::create(simdjson::ondemand::object arr) {
  return create_json(std::move(std::make_unique<SimdObject>(arr)));
}
