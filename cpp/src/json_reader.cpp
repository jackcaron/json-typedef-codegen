#include "json_reader.hpp"
#include "internal.hpp"
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

    // - - -
    BaseObjectIterator::~BaseObjectIterator() {}
    ObjectIterator::~ObjectIterator() {}
    JsonObjectIterator
    BaseObjectIterator::create_json(ObjectIteratorPtr&& pimpl) {
      return JsonObjectIterator(std::move(pimpl));
    }

    // - - -
    BaseArray::~BaseArray() {}
    Array::~Array() {}
    JsonArray BaseArray::create_json(ArrayPtr&& pimpl) {
      return JsonArray(std::move(pimpl));
    }

    // - - -
    BaseObject::~BaseObject() {}
    Object::~Object() {}
    JsonObject BaseObject::create_json(ObjectPtr&& pimpl) {
      return JsonObject(std::move(pimpl));
    }

    // - - -
    BaseValue::~BaseValue() {}
    Value::~Value() {}
    JsonValue BaseValue::create_json(ValuePtr&& pimpl) {
      return JsonValue(std::move(pimpl));
    }

    // - - -
    template <typename Base> struct Unbase;
    template <> struct Unbase<BaseArrayIterator> {
      using type = ArrayIterator;
    };
    template <> struct Unbase<BaseObjectIterator> {
      using type = ObjectIterator;
    };
    template <> struct Unbase<BaseArray> { using type = Array; };
    template <> struct Unbase<BaseObject> { using type = Object; };
    template <> struct Unbase<BaseValue> { using type = Value; };

    template <typename Base, typename UnbaseT = Unbase<Base>::type>
    constexpr const UnbaseT* unbase(const std::unique_ptr<Base>& base) {
      return dynamic_cast<const UnbaseT*>(base.get());
    }
    template <typename Base, typename UnbaseT = Unbase<Base>::type>
    constexpr UnbaseT* unbase(std::unique_ptr<Base>& base) {
      return dynamic_cast<UnbaseT*>(base.get());
    }

  } // namespace Specialization

  using namespace std::string_view_literals;
  namespace Spec = Specialization;

  // ------------------------------------------

  JsonArrayIterator::JsonArrayIterator(Spec::ArrayIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonArrayIterator::value_type
  JsonArrayIterator::operator*() const {
    if (m_pimpl) {
      return Spec::unbase(m_pimpl)->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  DLL_PUBLIC JsonArrayIterator& JsonArrayIterator::operator++() {
    if (m_pimpl) {
      Spec::unbase(m_pimpl)->next();
    }
    return *this;
  }

  DLL_PUBLIC bool JsonArrayIterator::operator==(std::default_sentinel_t) const {
    return !m_pimpl || Spec::unbase(m_pimpl)->done();
  }

  // ------------------------------------------

  JsonObjectIterator::JsonObjectIterator(Spec::ObjectIteratorPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonObjectIterator::value_type
  JsonObjectIterator::operator*() const {
    if (m_pimpl) {
      return Spec::unbase(m_pimpl)->get();
    }
    return makeJsonError(JsonErrorTypes::Invalid, "end iterator"sv);
  }

  DLL_PUBLIC JsonObjectIterator& JsonObjectIterator::operator++() {
    if (m_pimpl) {
      Spec::unbase(m_pimpl)->next();
    }
    return *this;
  }

  DLL_PUBLIC bool
  JsonObjectIterator::operator==(std::default_sentinel_t) const {
    return !m_pimpl || Spec::unbase(m_pimpl)->done();
  }

  // ------------------------------------------

  JsonArray::JsonArray(Spec::ArrayPtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonArrayIterator JsonArray::begin() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->begin() : JsonArrayIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  JsonObject::JsonObject(Spec::ObjectPtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonObjectIterator JsonObject::begin() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->begin() : JsonObjectIterator();
  }

  //    -   -   -   -   -   -   -   -   -   -
  static UnexpJsonError noPimpl() {
    return makeJsonError(JsonErrorTypes::Invalid, "invalid/empty JsonValue"sv);
  }

  JsonValue::JsonValue(Spec::ValuePtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonTypes JsonValue::get_type() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->get_type() : JsonTypes::Invalid;
  }

  DLL_PUBLIC ExpType<bool> JsonValue::is_null() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->is_null() : noPimpl();
  }
  DLL_PUBLIC ExpType<bool> JsonValue::read_bool() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_bool() : noPimpl();
  }
  DLL_PUBLIC ExpType<double> JsonValue::read_double() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_double() : noPimpl();
  }
  DLL_PUBLIC ExpType<uint64_t> JsonValue::read_u64() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_u64() : noPimpl();
  }
  DLL_PUBLIC ExpType<int64_t> JsonValue::read_i64() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_i64() : noPimpl();
  }
  DLL_PUBLIC ExpType<std::string> JsonValue::read_str() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_str() : noPimpl();
  }
  DLL_PUBLIC ExpType<JsonArray> JsonValue::read_array() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_array() : noPimpl();
  }
  DLL_PUBLIC ExpType<JsonObject> JsonValue::read_object() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_object() : noPimpl();
  }

} // namespace JsonTypedefCodeGen::Reader