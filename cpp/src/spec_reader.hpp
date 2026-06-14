#pragma once

#include "json_reader.hpp"

// internal specialization for each library
namespace JsonTypedefCodeGen::Reader::Specialization {

  class ArrayIterator : public BaseArrayIterator {
  public:
    virtual ~ArrayIterator();

    [[nodiscard]] virtual ExpType<JsonValue> get() const = 0;
    virtual void next() = 0;
    virtual bool done() const = 0;
  };

  class ObjectIterator : public BaseObjectIterator {
  public:
    virtual ~ObjectIterator();

    [[nodiscard]] virtual ExpType<ObjectIteratorPair> get() const = 0;
    virtual void next() = 0;
    virtual bool done() const = 0;
  };

  class Array : public BaseArray {
  public:
    virtual ~Array();

    [[nodiscard]] virtual JsonArrayIterator begin() const = 0;
  };

  class Object : public BaseObject {
  public:
    virtual ~Object();

    [[nodiscard]] virtual JsonObjectIterator begin() const = 0;
  };

  class Value : public BaseValue {
  public:
    virtual ~Value();

    [[nodiscard]] virtual JsonTypes get_type() const = 0;

    [[nodiscard]] virtual ExpType<bool> is_null() const = 0;
    [[nodiscard]] virtual ExpType<bool> read_bool() const = 0;
    [[nodiscard]] virtual ExpType<double> read_double() const = 0;
    [[nodiscard]] virtual ExpType<uint64_t> read_u64() const = 0;
    [[nodiscard]] virtual ExpType<int64_t> read_i64() const = 0;
    [[nodiscard]] virtual ExpType<std::string> read_str() const = 0;
    [[nodiscard]] virtual ExpType<JsonArray> read_array() const = 0;
    [[nodiscard]] virtual ExpType<JsonObject> read_object() const = 0;

    [[nodiscard]] virtual NumberType get_number_type() const = 0;
  };

} // namespace JsonTypedefCodeGen::Reader::Specialization
