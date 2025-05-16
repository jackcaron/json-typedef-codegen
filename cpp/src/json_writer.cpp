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
    AbsSerializer::write_number(const Data::JsonValue& val) {
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

    ExpType<ExpType<void>>
    AbsSerializer::write_val(const Data::JsonValue& val) {
      switch (val.get_type()) {
      case JsonTypes::Null:
        return write_null();

      case JsonTypes::Bool:
        return opt_to_exp(val.read_bool()).transform([&](bool b) {
          return write_bool(b);
        });

      case JsonTypes::Number:
        return write_number(val);

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

    ExpType<void> AbsSerializer::write_key_val(const std::string_view key,
                                               const Data::JsonValue& val) {
      return chain_exec_void_expected(
          [&]() {
            return write_key(key);
          },
          [&]() {
            return write(val);
          });
    }

    ExpType<void> AbsSerializer::write(const Data::JsonArray& arr) {
      auto w_item = [&](auto item) {
        return write(item);
      };
      return chain_exec_void_expected(
          start_array_exec(),
          [&]() {
            return arr.for_each(w_item);
          },
          end_array_exec());
    }

    ExpType<void> AbsSerializer::write(const Data::JsonObject& obj) {
      auto key_val = [&](auto key, auto val) {
        return write_key_val(key, val);
      };
      return chain_exec_void_expected(
          start_object_exec(),
          [&]() {
            return obj.for_each(key_val);
          },
          end_object_exec());
    }

    ExpType<void> AbsSerializer::write(const Data::JsonValue& val) {
      return flatten_expected(write_val(val));
    }

    constexpr const AbsSerializer* unbase(const SerializerPtr& base) {
      return dynamic_cast<const AbsSerializer*>(base.get());
    }
    constexpr AbsSerializer* unbase(SerializerPtr& base) {
      return dynamic_cast<AbsSerializer*>(base.get());
    }

    //   -   -   -   -   -   -   -   -   -   -   -   -

    StateBaseSerializer::StateBaseSerializer(const States init_state)
        : m_states({init_state}) {}
    StateBaseSerializer::~StateBaseSerializer() {}

    ExpType<void> StateBaseSerializer::can_start_object() const {
      switch (state()) {
      case States::RootObject:
      case States::Object:
        return make_json_error(
            JsonErrorTypes::Invalid,
            "a key is required to create an object inside base object"sv);

      default:
        return ExpType<void>();
      }
    }

    ExpType<void> StateBaseSerializer::can_end_object() const {
      switch (state()) {
      case States::RootArray:
      case States::Array:
        return make_json_error(JsonErrorTypes::Invalid,
                               "cannot end an array as an object"sv);

      case States::RootObject:
        return make_json_error(JsonErrorTypes::Invalid,
                               "cannot end root object"sv);

      case States::Object:
      default:
        return ExpType<void>();

      case States::ObjectKey:
        return make_json_error(JsonErrorTypes::Invalid,
                               "closing an object before resolving a key"sv);
      }
    }

    ExpType<void> StateBaseSerializer::can_start_array() const {
      switch (state()) {
      case States::RootObject:
      case States::Object:
        return make_json_error(
            JsonErrorTypes::Invalid,
            "a key is required to create an array inside an object"sv);

      default:
        return ExpType<void>();
      }
    }

    ExpType<void> StateBaseSerializer::can_end_array() const {
      switch (state()) {
      case States::RootArray:
        return make_json_error(JsonErrorTypes::Invalid,
                               "cannot end root array"sv);

      case States::Array:
        return ExpType<void>();

      default:
        return make_json_error(JsonErrorTypes::Invalid,
                               "cannot end an object as an array"sv);
      }
    }

    ExpType<void> StateBaseSerializer::write_key(const std::string_view key) {
      switch (state()) {
      case States::RootObject:
      case States::Object:
      default:
        push_state(States::ObjectKey);
        push_key(key);
        return ExpType<void>();

      case States::ObjectKey:
        return make_json_error(JsonErrorTypes::Invalid,
                               "object already has a key"sv);

      case States::RootArray:
      case States::Array:
        return make_json_error(JsonErrorTypes::Invalid,
                               "cannot write a key in an array"sv);
      }
    }

  } // namespace Specialization

  namespace Spec = Specialization;

  // ------------------------------------------

  Serializer::Serializer(Spec::SerializerPtr&& pimpl)
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
