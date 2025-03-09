#include "array.hpp"

#include "value.hpp"

ExpType<JsonValue> NlohArrayIterator::get() const {
  return NlohValue::create(*m_iter);
}

void NlohArrayIterator::next() {
  if (m_iter != m_end) {
    ++m_iter;
  }
}

bool NlohArrayIterator::done() const { return m_iter == m_end; }

JsonArrayIterator NlohArrayIterator::create(NlohVectorIter begin,
                                            NlohVectorIter end) {
  return create_json(
      std::move(std::make_unique<NlohArrayIterator>(begin, end)));
}

// -------------------------------------------
JsonArrayIterator NlohArray::begin() const {
  auto first = m_array.begin(), last = m_array.end();
  return NlohArrayIterator::create(first, last);
}

JsonArray NlohArray::create(const NlohArray arr) {
  return create_json(std::move(std::make_unique<NlohArray>(arr)));
}
