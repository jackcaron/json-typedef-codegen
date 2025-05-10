#include "json_writer.hpp"
#include "internal.hpp"
#include "spec_writer.hpp"

using namespace std::string_view_literals;

namespace JsonTypedefCodeGen::Writer {

  namespace {

    constexpr UnexpJsonError no_pimpl() {
      return make_json_error(JsonErrorTypes::Invalid,
                             "invalid/empty Serializer"sv);
    }

    template <typename Type> struct OptToExp;
    template <> struct OptToExp<bool> {
      static constexpr auto error = "expected a boolean"sv;
    };
    template <> struct OptToExp<double> {
      static constexpr auto error = "expected a double"sv;
    };
    template <> struct OptToExp<uint64_t> {
      static constexpr auto error = "expected an unsigned integer"sv;
    };
    template <> struct OptToExp<int64_t> {
      static constexpr auto error = "expected a signed integer"sv;
    };
    template <> struct OptToExp<std::string_view> {
      static constexpr auto error = "expected a string"sv;
    };
    template <> struct OptToExp<Data::JsonArray> {
      static constexpr auto error = "expected an array"sv;
    };
    template <> struct OptToExp<Data::JsonObject> {
      static constexpr auto error = "expected an object"sv;
    };

    template <typename Type>
    constexpr ExpType<Type> opt_to_exp(std::optional<Type> opt) {
      if (opt.has_value()) {
        return std::move(opt.value());
      } else {
        return make_json_error(JsonErrorTypes::WrongType,
                               OptToExp<Type>::error);
      }
    }

  } // namespace

  namespace Specialization {

    BaseSerializer::~BaseSerializer() {}
    AbsSerializer::~AbsSerializer() {}
    Serializer BaseSerializer::create_serializer(SerializerPtr&& pimpl) {
      return Serializer(std::move(pimpl));
    }

    ExpType<ExpType<void>>
    AbsSerializer::_write_number(const Data::JsonValue& val) {
      switch (val.get_number_type()) {
      case NumberType::Double:
        return opt_to_exp(val.read_double()).transform([&](auto d) {
          return write_double(d);
        });
      case NumberType::U64:
        return opt_to_exp(val.read_u64()).transform([&](auto u) {
          return write_u64(u);
        });
      case NumberType::I64:
        return opt_to_exp(val.read_i64()).transform([&](auto i) {
          return write_i64(i);
        });
      case NumberType::NaN:
      default:
        return make_json_error(JsonErrorTypes::Number);
      }
    }

    ExpType<ExpType<void>> AbsSerializer::_write(const Data::JsonValue& val) {
      switch (val.get_type()) {
      case JsonTypes::Null:
        return write_null();

      case JsonTypes::Bool:
        return opt_to_exp(val.read_bool()).transform([&](bool b) {
          return write_bool(b);
        });

      case JsonTypes::Number:
        return _write_number(val);

      case JsonTypes::Array:
        return opt_to_exp(val.read_array()).transform([&](auto a) {
          return write(a);
        });

      case JsonTypes::Object:
        return opt_to_exp(val.read_object()).transform([&](auto o) {
          return write(o);
        });

      case JsonTypes::String:
        return opt_to_exp(val.read_str()).transform([&](auto str) {
          return write_str(str);
        });

      case JsonTypes::Invalid:
      default:
        return make_json_error(JsonErrorTypes::Invalid);
      }
    }

    ExpType<void> AbsSerializer::write(const Data::JsonArray& arr) {
      return chain_void_expected(    //
          start_array(),             //
          Data::json_array_for_each( //
              arr,
              [&](const auto& item) {
                return write(item);
              }),
          end_array());
    }

    ExpType<void> AbsSerializer::write(const Data::JsonObject& obj) {
      return chain_void_expected(     //
          start_object(),             //
          Data::json_object_for_each( //
              obj,
              [&](const auto key, const auto& item) {
                return chain_void_expected(write_key(key), write(item));
              }),
          end_object());
    }

    ExpType<void> AbsSerializer::write(const Data::JsonValue& val) {
      return flatten_expected(_write(val));
    }

    constexpr const AbsSerializer* unbase(const SerializerPtr& base) {
      return dynamic_cast<const AbsSerializer*>(base.get());
    }
    constexpr AbsSerializer* unbase(SerializerPtr& base) {
      return dynamic_cast<AbsSerializer*>(base.get());
    }

  } // namespace Specialization

  namespace Spec = Specialization;

  // ------------------------------------------

  Serializer::Serializer(Specialization::SerializerPtr&& pimpl)
      : m_pimpl(std::move(pimpl)) {}

  DLL_PUBLIC Serializer::~Serializer() { close(); }

  DLL_PUBLIC ExpType<void> Serializer::close() {
    if (m_pimpl) {
      auto res = Spec::unbase(m_pimpl)->close();
      m_pimpl.reset();
      return res;
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<void> Serializer::write_null() {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_null() : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_bool(const bool b) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_bool(b) : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_double(const double d) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_double(d) : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_i64(const int64_t i) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_i64(i) : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_u64(const uint64_t u) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_u64(u) : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_str(const std::string_view str) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_str(str) : no_pimpl();
  }

  DLL_PUBLIC ExpType<void> Serializer::start_object() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_object() : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::write_key(const std::string_view key) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write_key(key) : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::end_object() {
    return m_pimpl ? Spec::unbase(m_pimpl)->end_object() : no_pimpl();
  }

  DLL_PUBLIC ExpType<void> Serializer::start_array() {
    return m_pimpl ? Spec::unbase(m_pimpl)->start_array() : no_pimpl();
  }
  DLL_PUBLIC ExpType<void> Serializer::end_array() {
    return m_pimpl ? Spec::unbase(m_pimpl)->end_array() : no_pimpl();
  }

  // ------------------------------------------
  DLL_PUBLIC ExpType<void> Serializer::write(const Data::JsonArray& arr) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write(arr) : no_pimpl();
  }

  DLL_PUBLIC ExpType<void> Serializer::write(const Data::JsonObject& obj) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write(obj) : no_pimpl();
  }

  DLL_PUBLIC ExpType<void> Serializer::write(const Data::JsonValue& val) {
    return m_pimpl ? Spec::unbase(m_pimpl)->write(val) : no_pimpl();
  }

} // namespace JsonTypedefCodeGen::Writer
