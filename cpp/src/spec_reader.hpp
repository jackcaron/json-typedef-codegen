#pragma once

#include "json_reader.hpp"

namespace JsonTypedefCodeGen::Reader {

  // internal specialization for each library
  namespace Specialization {

    class ArrayIterator {
    public:
      virtual ~ArrayIterator();

      virtual ExpType<JsonValue> get() const = 0;
      virtual void next() = 0;
      virtual bool identical(const ArrayIterator* rhs = nullptr) const = 0;
    };

    class ObjectIterator {
    public:
      virtual ~ObjectIterator();

      virtual ExpType<ObjectIteratorPair> get() const = 0;
      virtual void next() = 0;
      virtual bool identical(const ObjectIterator* rhs = nullptr) const = 0;
    };

    class Array {
    public:
      virtual ~Array();

      virtual JsonArrayIterator begin() const = 0;
      virtual JsonArrayIterator end() const = 0;
    };

    class Object {
    public:
      virtual ~Object();

      virtual JsonObjectIterator begin() const = 0;
      virtual JsonObjectIterator end() const = 0;
    };

    class Value {
    public:
      virtual ~Value();

      virtual JsonTypes get_type() const = 0;

      virtual ExpType<bool> is_null() const = 0;
      virtual ExpType<bool> read_bool() const = 0;
      virtual ExpType<double> read_double() const = 0;
      virtual ExpType<uint32_t> read_u32() const = 0;
      virtual ExpType<int32_t> read_i32() const = 0;
      virtual ExpType<std::string> read_str() const = 0;
      virtual ExpType<JsonArray> read_array() const = 0;
      virtual ExpType<JsonObject> read_object() const = 0;
    };

  } // namespace Specialization
} // namespace JsonTypedefCodeGen::Reader
