#include "json_reader.hpp"
#include "spec_reader.hpp"

namespace JsonTypedefCodeGen::Reader {

  namespace Specialization {

    ArrayIterator::~ArrayIterator() {}

    ObjectIterator::~ObjectIterator() {}

    Array::~Array() {}
    Object::~Object() {}
    Value::~Value() {}

  } // namespace Specialization

  using namespace std::string_view_literals;

  // ------------------------------------------

  JsonArrayIterator::JsonArrayIterator(Specialization::ArrayIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  JsonArrayIterator::~JsonArrayIterator() {}

  JsonArrayIterator::value_type JsonArrayIterator::operator*() const {
    if (m_pimpl) {
      return m_pimpl->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  JsonArrayIterator& JsonArrayIterator::operator++() {
    if (m_pimpl) {
      m_pimpl->next();
    }
    return *this;
  }

  bool JsonArrayIterator::operator==(const JsonArrayIterator& rhs) const {
    return m_pimpl ? m_pimpl->identical(rhs.m_pimpl.get())
                   : (!rhs.m_pimpl || rhs.m_pimpl->identical());
  }

  // ------------------------------------------

  JsonObjectIterator::JsonObjectIterator(
      Specialization::ObjectIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  JsonObjectIterator::~JsonObjectIterator() {}

  JsonObjectIterator::value_type JsonObjectIterator::operator*() const {
    if (m_pimpl) {
      return m_pimpl->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  JsonObjectIterator& JsonObjectIterator::operator++() {
    if (m_pimpl) {
      m_pimpl->next();
    }
    return *this;
  }

  bool JsonObjectIterator::operator==(const JsonObjectIterator& rhs) const {
    return m_pimpl ? m_pimpl->identical(rhs.m_pimpl.get())
                   : (!rhs.m_pimpl || rhs.m_pimpl->identical());
  }

  // ------------------------------------------

  JsonArray::JsonArray(Specialization::ArrayPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}
  JsonArray::~JsonArray() {}

  JsonArrayIterator JsonArray::begin() const {
    return m_pimpl ? m_pimpl->begin() : JsonArrayIterator();
  }
  JsonArrayIterator JsonArray::end() const {
    return m_pimpl ? m_pimpl->end() : JsonArrayIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  JsonObject::JsonObject(Specialization::ObjectPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}
  JsonObject::~JsonObject() {}

  JsonObjectIterator JsonObject::begin() const {
    return m_pimpl ? m_pimpl->begin() : JsonObjectIterator();
  }
  JsonObjectIterator JsonObject::end() const {
    return m_pimpl ? m_pimpl->end() : JsonObjectIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  static UnexpJsonError noPimpl() {
    return makeJsonError(JsonErrorTypes::Invalid, "invalid/empty JsonValue"sv);
  }

  JsonValue::JsonValue(Specialization::ValuePtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}
  JsonValue::~JsonValue() {}

  JsonTypes JsonValue::get_type() const {
    return m_pimpl ? m_pimpl->get_type() : JsonTypes::Invalid;
  }

  ExpType<bool> JsonValue::is_null() const {
    return m_pimpl ? m_pimpl->is_null() : noPimpl();
  }
  ExpType<bool> JsonValue::read_bool() const {
    return m_pimpl ? m_pimpl->read_bool() : noPimpl();
  }
  ExpType<double> JsonValue::read_double() const {
    return m_pimpl ? m_pimpl->read_double() : noPimpl();
  }
  ExpType<uint32_t> JsonValue::read_u32() const {
    return m_pimpl ? m_pimpl->read_u32() : noPimpl();
  }
  ExpType<int32_t> JsonValue::read_i32() const {
    return m_pimpl ? m_pimpl->read_i32() : noPimpl();
  }
  ExpType<std::string> JsonValue::read_str() const {
    return m_pimpl ? m_pimpl->read_str() : noPimpl();
  }
  ExpType<JsonArray> JsonValue::read_array() const {
    return m_pimpl ? m_pimpl->read_array() : noPimpl();
  }
  ExpType<JsonObject> JsonValue::read_object() const {
    return m_pimpl ? m_pimpl->read_object() : noPimpl();
  }

} // namespace JsonTypedefCodeGen::Reader