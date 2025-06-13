#include "json_reader.hpp"
#include "internal.hpp"
#include "spec_reader.hpp"

#include <format>

namespace JsonTypedefCodeGen::Reader {

  namespace Specialization {

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

    static ExpType<Data::JsonValue> clone_number(const Value* val) {
      constexpr auto conv = [](auto v) {
        return Data::JsonValue(v);
      };

      switch (val->get_number_type()) {
      case NumberType::Double:
        return val->read_double().transform(conv);
      case NumberType::I64:
        return val->read_i64().transform(conv);
      case NumberType::U64:
        return val->read_u64().transform(conv);
      case NumberType::NaN:
      default:
        return make_json_error(JsonErrorTypes::WrongType);
      }
    }

    // - - -
    template <typename Base> struct Unbase;
    template <> struct Unbase<BaseArrayIterator> {
      using type = ArrayIterator;
    };
    template <> struct Unbase<BaseObjectIterator> {
      using type = ObjectIterator;
    };
    template <> struct Unbase<BaseArray> {
      using type = Array;
    };
    template <> struct Unbase<BaseObject> {
      using type = Object;
    };
    template <> struct Unbase<BaseValue> {
      using type = Value;
    };

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
    return make_json_error(JsonErrorTypes::Invalid, "end iterator"sv);
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
    return make_json_error(JsonErrorTypes::Invalid, "end iterator"sv);
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

  DLL_PUBLIC ExpType<Data::JsonArray> JsonArray::clone() const {
    Data::JsonArray _result;
    auto& result = _result.internal();
    for (const auto& item : *this) {
      if (!item.has_value()) [[unlikely]] {
        return std::unexpected(item.error());
      }

      if (const auto tmp = item.value().clone(); tmp.has_value()) [[likely]] {
        result.emplace_back(std::move(tmp.value()));
      } else {
        return std::unexpected(tmp.error());
      }
    }
    return _result;
  }

  // ------------------------------------------
  JsonObject::JsonObject(Spec::ObjectPtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonObjectIterator JsonObject::begin() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->begin() : JsonObjectIterator();
  }

  DLL_PUBLIC ExpType<Data::JsonObject> JsonObject::clone() const {
    Data::JsonObject _result;
    auto& result = _result.internal();
    for (const auto& item : *this) {
      if (!item.has_value()) [[unlikely]] {
        return std::unexpected(item.error());
      }

      const auto& [key, val] = item.value();
      if (const auto tmp = val.clone(); tmp.has_value()) [[likely]] {
        const auto [it, ok] = result.insert({key, tmp.value()});
        if (!ok) {
          const auto err = format("Duplicated key {}", key);
          return make_json_error(JsonErrorTypes::String, err);
        }
      } else {
        return std::unexpected(item.error());
      }
    }
    return _result;
  }

  // ------------------------------------------
  static UnexpJsonError no_pimpl() {
    return make_json_error(JsonErrorTypes::Invalid,
                           "invalid/empty JsonValue"sv);
  }

  JsonValue::JsonValue(Spec::ValuePtr&& pimpl) : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC JsonTypes JsonValue::get_type() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->get_type() : JsonTypes::Invalid;
  }

  DLL_PUBLIC ExpType<bool> JsonValue::is_null() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->is_null() : no_pimpl();
  }
  DLL_PUBLIC ExpType<bool> JsonValue::read_bool() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_bool() : no_pimpl();
  }
  DLL_PUBLIC ExpType<double> JsonValue::read_double() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_double() : no_pimpl();
  }
  DLL_PUBLIC ExpType<uint64_t> JsonValue::read_u64() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_u64() : no_pimpl();
  }
  DLL_PUBLIC ExpType<int64_t> JsonValue::read_i64() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_i64() : no_pimpl();
  }
  DLL_PUBLIC ExpType<std::string> JsonValue::read_str() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_str() : no_pimpl();
  }
  DLL_PUBLIC ExpType<JsonArray> JsonValue::read_array() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_array() : no_pimpl();
  }
  DLL_PUBLIC ExpType<JsonObject> JsonValue::read_object() const {
    return m_pimpl ? Spec::unbase(m_pimpl)->read_object() : no_pimpl();
  }

  DLL_PUBLIC ExpType<Data::JsonValue> JsonValue::clone() const {
    constexpr auto conv = [](auto v) {
      return Data::JsonValue(v);
    };

    switch (get_type()) {
    case JsonTypes::Null:
      return Data::JsonValue(nullptr);

    case JsonTypes::Bool:
      return read_bool().transform(conv);

    case JsonTypes::Number:
      return clone_number(Spec::unbase(m_pimpl));

    case JsonTypes::String:
      return read_str().transform(conv);

    case JsonTypes::Array: {
      if (auto tmp = read_array(); tmp.has_value()) {
        return tmp.value().clone().transform(conv);
      } else {
        return std::unexpected(tmp.error());
      }
    } break;

    case JsonTypes::Object: {
      if (auto tmp = read_object(); tmp.has_value()) {
        return tmp.value().clone().transform(conv);
      } else {
        return std::unexpected(tmp.error());
      }
    } break;

    default:
      break;
    }
    return make_json_error(JsonErrorTypes::Invalid);
  }

  DLL_PUBLIC ExpType<void> json_array_for_each(const JsonArray& array,
                                               ArrayForEachFn cb) {
    for (auto item : array) {
      if (auto exp = flatten_expected(item.transform(cb)); !exp.has_value()) {
        return UnexpJsonError(exp.error());
      }
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> json_array_for_each(const JsonValue& value,
                                               ArrayForEachFn cb) {
    return flatten_expected(
        value.read_array().transform([&cb](const auto& array) {
          return json_array_for_each(array, cb);
        }));
  }

  DLL_PUBLIC ExpType<void> json_object_for_each(const JsonObject& object,
                                                ObjectForEachFn cb) {
    for (auto item : object) {
      auto exp = flatten_expected(item.transform([&cb](auto& pair) {
        const auto [key, val] = std::move(pair);
        return cb(key, val);
      }));
      if (!exp.has_value()) {
        return UnexpJsonError(exp.error());
      }
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> json_object_for_each(const JsonValue& value,
                                                ObjectForEachFn cb) {
    return flatten_expected(
        value.read_object().transform([&cb](const auto& object) {
          return json_object_for_each(object, cb);
        }));
  }

} // namespace JsonTypedefCodeGen::Reader