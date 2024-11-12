#include "array.hpp"

#include "value.hpp"

ExpType<JsonValue> SimdArrayIterator::get() const {
  auto err_type = m_iter.error();
  if (err_type == simdjson::SUCCESS) {
    auto unsafe_val = m_iter.value_unsafe();
    auto val = *unsafe_val;
    err_type = val.error();
    if (err_type == simdjson::SUCCESS) {
      return SimdValue::create(val.value_unsafe());
    }
  }
  return makeJsonError(err_type);
}

void SimdArrayIterator::next() {
  if (m_iter.error() == simdjson::SUCCESS) {
    if (m_iter.value_unsafe() == m_end) {
      // if it reached the end, invalidate it
      m_iter = ArrIterResult(simdjson::OUT_OF_BOUNDS);
    } else {
      ++(m_iter.value_unsafe());
    }
  }
}

bool SimdArrayIterator::done() const {
  return m_iter.error() != simdjson::SUCCESS || m_iter.value_unsafe() == m_end;
}

JsonArrayIterator SimdArrayIterator::create(ArrIterResult begin, ArrIter end) {
  return create_json(
      std::move(std::make_unique<SimdArrayIterator>(begin, end)));
}

// -------------------------------------------
JsonArrayIterator SimdArray::begin() const {
  auto first = m_array.begin(), last = m_array.end();
  ArrIter end; // default and invalid, never used if there's an error

  if (const auto err_type = last.error(); err_type != simdjson::SUCCESS) {
    first = ArrIterResult(err_type); // forward the error on the first iterator
  } else {
    end = last.value_unsafe();
  }
  return SimdArrayIterator::create(first, end);
}

JsonArray SimdArray::create(simdjson::ondemand::array& arr) {
  return create_json(std::move(std::make_unique<SimdArray>(arr)));
}
