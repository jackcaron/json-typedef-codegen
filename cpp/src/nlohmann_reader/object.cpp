#include "object.hpp"

#include "value.hpp"

ExpType<ObjectIteratorPair> NlohObjectIterator::get() const {
  return ObjectIteratorPair{m_iter->first, NlohValue::create(m_iter->second)};
}

void NlohObjectIterator::next() {
  if (m_iter != m_end) {
    ++m_iter;
  }
}

bool NlohObjectIterator::done() const { return m_iter == m_end; }

JsonObjectIterator NlohObjectIterator::create(NlohMapIter begin,
                                              NlohMapIter end) {
  return create_json(
      std::move(std::make_unique<NlohObjectIterator>(begin, end)));
}

// -------------------------------------------
JsonObjectIterator NlohObject::begin() const {
  auto first = m_object.begin(), last = m_object.end();
  return NlohObjectIterator::create(first, last);
}

JsonObject NlohObject::create(const NlohMap obj) {
  return create_json(std::move(std::make_unique<NlohObject>(obj)));
}
