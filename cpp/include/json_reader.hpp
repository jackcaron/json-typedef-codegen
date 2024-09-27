#pragma once

#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>

#include "common.hpp"

namespace JsonTypedefCodeGen::Reader {

  class JsonArray;
  class JsonObject;
  class JsonValue;

  class JsonArrayIterator;
  class JsonObjectIterator;
  using ObjectIteratorPair = std::pair<std::string, JsonValue>;

  namespace Specialization {

    class ArrayIterator;
    using ArrayIteratorPtr = std::unique_ptr<ArrayIterator>;

    class ObjectIterator;
    using ObjectIteratorPtr = std::unique_ptr<ObjectIterator>;

    class Array;
    using ArrayPtr = std::unique_ptr<Array>;

    class Object;
    using ObjectPtr = std::unique_ptr<Object>;

    class Value;
    using ValuePtr = std::unique_ptr<Value>;

  } // namespace Specialization

  class JsonArrayIterator : public std::input_iterator_tag {
  private:
    Specialization::ArrayIteratorPtr m_pimpl;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = ExpType<JsonValue>;

    JsonArrayIterator() = default;
    JsonArrayIterator(const JsonArrayIterator&) = delete;
    JsonArrayIterator(JsonArrayIterator&&) = default;
    JsonArrayIterator(Specialization::ArrayIteratorPtr&& pimpl);
    ~JsonArrayIterator();

    JsonArrayIterator& operator=(const JsonArrayIterator&) = delete;
    JsonArrayIterator& operator=(JsonArrayIterator&&) = default;

    value_type operator*() const;
    JsonArrayIterator& operator++();
    inline void operator++(int) { ++(*this); }

    bool operator==(const JsonArrayIterator& rhs) const;
    inline bool operator!=(const JsonArrayIterator& rhs) const {
      return !((*this) == rhs);
    }
  };

  class JsonObjectIterator : public std::input_iterator_tag {
  private:
    Specialization::ObjectIteratorPtr m_pimpl;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = ExpType<ObjectIteratorPair>;

    JsonObjectIterator() = default;
    JsonObjectIterator(const JsonObjectIterator&) = delete;
    JsonObjectIterator(JsonObjectIterator&&) = default;
    JsonObjectIterator(Specialization::ObjectIteratorPtr&& pimpl);
    ~JsonObjectIterator();

    JsonObjectIterator& operator=(const JsonObjectIterator&) = delete;
    JsonObjectIterator& operator=(JsonObjectIterator&&) = default;

    value_type operator*() const;
    JsonObjectIterator& operator++();
    inline void operator++(int) { ++(*this); }

    bool operator==(const JsonObjectIterator& rhs) const;
    inline bool operator!=(const JsonObjectIterator& rhs) const {
      return !((*this) == rhs);
    }
  };

  class JsonArray {
  private:
    Specialization::ArrayPtr m_pimpl;

  public:
    JsonArray() = default;
    JsonArray(const JsonArray&) = delete;
    JsonArray(JsonArray&&) = default;
    JsonArray(Specialization::ArrayPtr&& pimpl);
    virtual ~JsonArray();

    JsonArray& operator=(const JsonArray&) = delete;
    JsonArray& operator=(JsonArray&&) = default;

    JsonArrayIterator begin() const;
    JsonArrayIterator end() const;
  };

  class JsonObject {
  private:
    Specialization::ObjectPtr m_pimpl;

  public:
    JsonObject() = default;
    JsonObject(const JsonObject&) = delete;
    JsonObject(JsonObject&&) = default;
    JsonObject(Specialization::ObjectPtr&& pimpl);
    virtual ~JsonObject();

    JsonObject& operator=(const JsonObject&) = delete;
    JsonObject& operator=(JsonObject&&) = default;

    JsonObjectIterator begin() const;
    JsonObjectIterator end() const;
  };

  class JsonValue {
  private:
    Specialization::ValuePtr m_pimpl;

  public:
    JsonValue() = default;
    JsonValue(const JsonValue&) = delete;
    JsonValue(JsonValue&&) = default;
    JsonValue(Specialization::ValuePtr&& pimpl);
    virtual ~JsonValue();

    JsonValue& operator=(const JsonValue&) = delete;
    JsonValue& operator=(JsonValue&&) = default;

    JsonTypes get_type() const;

    ExpType<bool> is_null() const;
    ExpType<bool> read_bool() const;
    ExpType<double> read_double() const;
    ExpType<uint32_t> read_u32() const;
    ExpType<int32_t> read_i32() const;
    ExpType<std::string> read_str() const;
    ExpType<JsonArray> read_array() const;
    ExpType<JsonObject> read_object() const;
  };

} // namespace JsonTypedefCodeGen::Reader
