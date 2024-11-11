#pragma once

#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>

#include "common.hpp"

#ifdef USE_SIMD
#include "simd_json/simdjson.h"
#endif

namespace JsonTypedefCodeGen::Reader {

  class JsonArrayIterator;
  class JsonObjectIterator;
  class JsonArray;
  class JsonObject;
  class JsonValue;

  namespace Specialization {

    class BaseArrayIterator;
    using ArrayIteratorPtr = std::unique_ptr<BaseArrayIterator>;

    class BaseObjectIterator;
    using ObjectIteratorPtr = std::unique_ptr<BaseObjectIterator>;

    class BaseArray;
    using ArrayPtr = std::unique_ptr<BaseArray>;

    class BaseObject;
    using ObjectPtr = std::unique_ptr<BaseObject>;

    class BaseValue;
    using ValuePtr = std::unique_ptr<BaseValue>;

    class BaseArrayIterator {
    protected:
      static JsonArrayIterator create_json(ArrayIteratorPtr&& pimpl);

    public:
      virtual ~BaseArrayIterator();
    };

    class BaseObjectIterator {
    protected:
      static JsonObjectIterator create_json(ObjectIteratorPtr&& pimpl);

    public:
      virtual ~BaseObjectIterator();
    };

    class BaseArray {
    protected:
      static JsonArray create_json(ArrayPtr&& pimpl);

    public:
      virtual ~BaseArray();
    };

    class BaseObject {
    protected:
      static JsonObject create_json(ObjectPtr&& pimpl);

    public:
      virtual ~BaseObject();
    };

    class BaseValue {
    protected:
      static JsonValue create_json(ValuePtr&& pimpl);

    public:
      virtual ~BaseValue();
    };

  } // namespace Specialization

  using ObjectIteratorPair = std::pair<std::string, JsonValue>;

  class JsonArrayIterator : public std::input_iterator_tag {
  private:
    friend class Specialization::BaseArrayIterator;

    Specialization::ArrayIteratorPtr m_pimpl;
    JsonArrayIterator(Specialization::ArrayIteratorPtr&& pimpl);

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = ExpType<JsonValue>;

    JsonArrayIterator() = default;
    JsonArrayIterator(const JsonArrayIterator&) = delete;
    JsonArrayIterator(JsonArrayIterator&&) = default;
    ~JsonArrayIterator() {}

    JsonArrayIterator& operator=(const JsonArrayIterator&) = delete;
    JsonArrayIterator& operator=(JsonArrayIterator&&) = default;

    value_type operator*() const;
    JsonArrayIterator& operator++();
    inline void operator++(int) { ++(*this); }

    bool operator==(std::default_sentinel_t) const;
  };

  class JsonObjectIterator : public std::input_iterator_tag {
  private:
    friend class Specialization::BaseObjectIterator;

    Specialization::ObjectIteratorPtr m_pimpl;
    JsonObjectIterator(Specialization::ObjectIteratorPtr&& pimpl);

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = ExpType<ObjectIteratorPair>;

    JsonObjectIterator() = default;
    JsonObjectIterator(const JsonObjectIterator&) = delete;
    JsonObjectIterator(JsonObjectIterator&&) = default;
    ~JsonObjectIterator() {}

    JsonObjectIterator& operator=(const JsonObjectIterator&) = delete;
    JsonObjectIterator& operator=(JsonObjectIterator&&) = default;

    value_type operator*() const;
    JsonObjectIterator& operator++();
    inline void operator++(int) { ++(*this); }

    bool operator==(std::default_sentinel_t) const;
  };

  class JsonArray {
  private:
    friend class Specialization::BaseArray;

    Specialization::ArrayPtr m_pimpl;
    JsonArray(Specialization::ArrayPtr&& pimpl);

  public:
    JsonArray() = default;
    JsonArray(const JsonArray&) = delete;
    JsonArray(JsonArray&&) = default;
    ~JsonArray() {}

    JsonArray& operator=(const JsonArray&) = delete;
    JsonArray& operator=(JsonArray&&) = default;

    JsonArrayIterator begin() const;
    inline std::default_sentinel_t end() const {
      return std::default_sentinel_t{};
    }
  };

  class JsonObject {
  private:
    friend class Specialization::BaseObject;

    Specialization::ObjectPtr m_pimpl;
    JsonObject(Specialization::ObjectPtr&& pimpl);

  public:
    JsonObject() = default;
    JsonObject(const JsonObject&) = delete;
    JsonObject(JsonObject&&) = default;
    ~JsonObject() {}

    JsonObject& operator=(const JsonObject&) = delete;
    JsonObject& operator=(JsonObject&&) = default;

    JsonObjectIterator begin() const;
    inline std::default_sentinel_t end() const {
      return std::default_sentinel_t{};
    }
  };

  class JsonValue {
  private:
    friend class Specialization::BaseValue;

    Specialization::ValuePtr m_pimpl;
    JsonValue(Specialization::ValuePtr&& pimpl);

  public:
    JsonValue() = default;
    JsonValue(const JsonValue&) = delete;
    JsonValue(JsonValue&&) = default;
    ~JsonValue() {}

    JsonValue& operator=(const JsonValue&) = delete;
    JsonValue& operator=(JsonValue&&) = default;

    JsonTypes get_type() const;

    ExpType<bool> is_null() const;
    ExpType<bool> read_bool() const;
    ExpType<double> read_double() const;
    ExpType<uint64_t> read_u64() const;
    ExpType<int64_t> read_i64() const;
    ExpType<std::string> read_str() const;
    ExpType<JsonArray> read_array() const;
    ExpType<JsonObject> read_object() const;
  };

#ifdef USE_SIMD
  ExpType<JsonValue> simdjson_root_value(simdjson::ondemand::value& root);
#endif

} // namespace JsonTypedefCodeGen::Reader
