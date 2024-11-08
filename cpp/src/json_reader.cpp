#include "json_reader.hpp"
#include "spec_reader.hpp"

namespace JsonTypedefCodeGen::Reader {

  namespace Specialization {

    // NOTE make the "create_json" functions hidden in the SO file

    // - - -
    BaseArrayIterator::~BaseArrayIterator() {}
    ArrayIterator::~ArrayIterator() {}
    JsonArrayIterator BaseArrayIterator::create_json(ArrayIteratorPtr&& pimpl) {
      return JsonArrayIterator(std::move(pimpl));
    }
    constexpr auto unbase(const std::unique_ptr<BaseArrayIterator>& base) {
      return dynamic_cast<ArrayIterator*>(base.get());
    }

    // - - -
    BaseObjectIterator::~BaseObjectIterator() {}
    ObjectIterator::~ObjectIterator() {}
    JsonObjectIterator
    BaseObjectIterator::create_json(ObjectIteratorPtr&& pimpl) {
      return JsonObjectIterator(std::move(pimpl));
    }
    constexpr auto unbase(const std::unique_ptr<BaseObjectIterator>& base) {
      return dynamic_cast<ObjectIterator*>(base.get());
    }

    // - - -
    BaseArray::~BaseArray() {}
    Array::~Array() {}
    JsonArray BaseArray::create_json(ArrayPtr&& pimpl) {
      return JsonArray(std::move(pimpl));
    }
    constexpr auto unbase(const std::unique_ptr<BaseArray>& base) {
      return dynamic_cast<Array*>(base.get());
    }

    // - - -
    BaseObject::~BaseObject() {}
    Object::~Object() {}
    JsonObject BaseObject::create_json(ObjectPtr&& pimpl) {
      return JsonObject(std::move(pimpl));
    }
    constexpr auto unbase(const std::unique_ptr<BaseObject>& base) {
      return dynamic_cast<Object*>(base.get());
    }

    // - - -
    BaseValue::~BaseValue() {}
    Value::~Value() {}
    JsonValue BaseValue::create_json(ValuePtr&& pimpl) {
      return JsonValue(std::move(pimpl));
    }
    constexpr auto unbase(const std::unique_ptr<BaseValue>& base) {
      return dynamic_cast<Value*>(base.get());
    }

  } // namespace Specialization

  using namespace std::string_view_literals;
  namespace Spec = Specialization;

  // ------------------------------------------

  JsonArrayIterator::JsonArrayIterator(Spec::ArrayIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  JsonArrayIterator::value_type JsonArrayIterator::operator*() const {
    if (m_pimpl) {
      return Spec::unbase(m_pimpl)->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  JsonArrayIterator& JsonArrayIterator::operator++() {
    if (m_pimpl) {
      Spec::unbase(m_pimpl)->next();
    }
    return *this;
  }

  bool JsonArrayIterator::operator==(std::default_sentinel_t) const {
    return !m_pimpl || Spec::unbase(m_pimpl)->empty();
  }

  // ------------------------------------------

  JsonObjectIterator::JsonObjectIterator(Spec::ObjectIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  JsonObjectIterator::value_type JsonObjectIterator::operator*() const {
    if (m_pimpl) {
      return Spec::unbase(m_pimpl)->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  JsonObjectIterator& JsonObjectIterator::operator++() {
    if (m_pimpl) {
      Spec::unbase(m_pimpl)->next();
    }
    return *this;
  }

  bool JsonObjectIterator::operator==(std::default_sentinel_t) const {
    return !m_pimpl || Spec::unbase(m_pimpl)->empty();
  }

  // ------------------------------------------

  JsonArray::JsonArray(Spec::ArrayPtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  JsonArrayIterator JsonArray::begin() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->begin() : JsonArrayIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  JsonObject::JsonObject(Spec::ObjectPtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  JsonObjectIterator JsonObject::begin() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->begin() : JsonObjectIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  static UnexpJsonError noPimpl() {
    return makeJsonError(JsonErrorTypes::Invalid, "invalid/empty JsonValue"sv);
  }

  JsonValue::JsonValue(Spec::ValuePtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  JsonTypes JsonValue::get_type() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->get_type() : JsonTypes::Invalid;
  }

  ExpType<bool> JsonValue::is_null() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->is_null() : noPimpl();
  }
  ExpType<bool> JsonValue::read_bool() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_bool() : noPimpl();
  }
  ExpType<double> JsonValue::read_double() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_double() : noPimpl();
  }
  ExpType<uint32_t> JsonValue::read_u32() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_u32() : noPimpl();
  }
  ExpType<int32_t> JsonValue::read_i32() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_i32() : noPimpl();
  }
  ExpType<std::string> JsonValue::read_str() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_str() : noPimpl();
  }
  ExpType<JsonArray> JsonValue::read_array() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_array() : noPimpl();
  }
  ExpType<JsonObject> JsonValue::read_object() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_object() : noPimpl();
  }

} // namespace JsonTypedefCodeGen::Reader