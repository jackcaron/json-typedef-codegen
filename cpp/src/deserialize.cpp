#if defined(USE_SIMD) || defined(USE_IN_NAPI) || defined(USE_IN_NLOH)

#define IMPL_DESERIALIZE
#include "deserialize.hpp"
#include "internal.hpp"

#include <cmath>
#include <format>
#include <limits>

using namespace std::string_view_literals;

namespace JsonTypedefCodeGen::Deserialize {

  namespace Errors {

    DLL_PUBLIC UnexpJsonError duplicated_key(const strview key) {
      const auto err = std::format("Duplicated key \"{}\""sv, key);
      return make_json_error(JsonErrorTypes::String, err);
    }

    DLL_PUBLIC UnexpJsonError not_string(const strview name) {
      const auto err = std::format("Not a string for {}"sv, name);
      return make_json_error(JsonErrorTypes::Invalid, err);
    }

    UnexpJsonError invalid_value(const strview val, const strview name) {
      const auto err = std::format("Invalid value \"{}\" for {}"sv, val, name);
      return make_json_error(JsonErrorTypes::Invalid, err);
    }

    UnexpJsonError missing_key(const strview entry, const strview name) {
      const auto err = std::format("Missing key \"{}\" for {}"sv, entry, name);
      return make_json_error(JsonErrorTypes::String, err);
    }

  } // namespace Errors

  DLL_PUBLIC ExpType<int>
  get_enum_index_impl(const strview value,
                      const std::span<const strview> entries,
                      const strview name) {
    for (int index = 0; const auto entry : entries) {
      if (value == entry) {
        return index;
      }
      ++index;
    }
    return Errors::invalid_value(value, name);
  }

  DLL_PUBLIC ExpType<void>
  visited_mandatory(const std::span<const int> mandatory_indices,
                    const std::span<bool> visited,
                    const std::span<const strview> entries,
                    const strview name) {
    //
    for (const auto midx : mandatory_indices) {
      if (!visited[midx]) {
        return Errors::missing_key(entries[midx], name);
      }
    }
    return ExpType<void>();
  }

  DLL_PUBLIC ExpType<std::string> get_disc_value(const Data::JsonObject& object,
                                                 const strview disc,
                                                 const strview name) {
    auto& inner = object.internal();

    if (auto fnd = inner.find(std::string(disc)); fnd == inner.end()) {
      return Errors::missing_key(disc, name);
    } else {
      if (auto opt_str = fnd->second.read_str(); opt_str.has_value()) {
        return std::string(opt_str.value());
      }
    }

    const auto err = std::format("Expected string value for {}"sv, disc);
    return make_json_error(JsonErrorTypes::Invalid, err);
  }

  DLL_PUBLIC ExpType<int>
  get_value_index(const strview value, const std::span<const strview> entries,
                  const strview name) {
    for (int index = 0; const auto entry : entries) {
      if (value == entry) {
        return index;
      }
      ++index;
    }

    const auto err = std::format("Invalid key \"{}\" in {}"sv, value, name);
    return make_json_error(JsonErrorTypes::Invalid, err);
  }

  //  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

  static ExpType<int64_t> test_i64(const JDt::JsonValue& value) {
    return optional_to_exp_type(value.read_i64(), JsonErrorTypes::Invalid,
                                "Not a number"sv);
  }

  static ExpType<uint64_t> test_u64(const JDt::JsonValue& value) {
    return optional_to_exp_type(value.read_u64(), JsonErrorTypes::Invalid,
                                "Not a number"sv);
  }

  static ExpType<double> test_double(const JDt::JsonValue& value) {
    return optional_to_exp_type(value.read_double(), JsonErrorTypes::Invalid,
                                "Not a number"sv);
  }

  template <typename NumT>
  static ExpType<NumT> test_numerical_limits(const ExpType<int64_t> value) {
    constexpr int64_t _min = std::numeric_limits<NumT>::min(),
                      _max = std::numeric_limits<NumT>::max();
    if (value.has_value()) {
      const auto val = value.value();
      if (val < _min || val > _max) {
        return make_json_error(
            JsonErrorTypes::Number,
            std::format("Signed value {} outside limits {}:{}"sv, val, _min,
                        _max));
      }
      return ExpType<NumT>((NumT)val);
    }
    return UnexpJsonError(std::move(value.error()));
  }

  template <typename NumT>
  static ExpType<NumT> test_numerical_limits(const ExpType<uint64_t> value) {
    constexpr uint64_t _max = std::numeric_limits<NumT>::max();
    if (value.has_value()) {
      const auto val = value.value();
      if (val > _max) {
        return make_json_error(
            JsonErrorTypes::Number,
            std::format("Unsigned value {} is greater than {}"sv, val, _max));
      }
      return ExpType<NumT>((NumT)val);
    }
    return UnexpJsonError(std::move(value.error()));
  }

  static ExpType<float> test_numerical_limits(const ExpType<double> value) {
    constexpr double _min = std::numeric_limits<float>::lowest(),
                     _max = std::numeric_limits<float>::max(),
                     _eps = std::numeric_limits<float>::min();
    if (value.has_value()) {
      const auto val = value.value();
      if (val < _min || val > _max) {
        return make_json_error(
            JsonErrorTypes::Number,
            std::format("Double value {} outside of float limits {}:{}"sv, val,
                        _min, _max));
      } else if (std::fabs(val) < _eps) {
        return make_json_error(
            JsonErrorTypes::Number,
            std::format(
                "Double value {} outside of float zeroes limits {}:{}"sv, val,
                -_eps, _eps));
      }
      return ExpType<float>((float)val);
    }
    return UnexpJsonError(std::move(value.error()));
  }

  //  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

  DLL_PUBLIC ExpType<bool>
  Json<bool>::deserialize(const JDt::JsonValue& value) {
    return optional_to_exp_type(value.read_bool(), JsonErrorTypes::Invalid,
                                "Not a bool"sv);
  }

  DLL_PUBLIC ExpType<int8_t>
  Json<int8_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<int8_t>(value.read_i64());
  }
  DLL_PUBLIC ExpType<int8_t>
  Json<int8_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<int8_t>(test_i64(value));
  }

  DLL_PUBLIC ExpType<uint8_t>
  Json<uint8_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<uint8_t>(value.read_u64());
  }
  DLL_PUBLIC ExpType<uint8_t>
  Json<uint8_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<uint8_t>(test_u64(value));
  }

  DLL_PUBLIC ExpType<int16_t>
  Json<int16_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<int16_t>(value.read_i64());
  }
  DLL_PUBLIC ExpType<int16_t>
  Json<int16_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<int16_t>(test_i64(value));
  }

  DLL_PUBLIC ExpType<uint16_t>
  Json<uint16_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<uint16_t>(value.read_u64());
  }
  DLL_PUBLIC ExpType<uint16_t>
  Json<uint16_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<uint16_t>(test_u64(value));
  }

  DLL_PUBLIC ExpType<int32_t>
  Json<int32_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<int32_t>(value.read_i64());
  }
  DLL_PUBLIC ExpType<int32_t>
  Json<int32_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<int32_t>(test_i64(value));
  }

  DLL_PUBLIC ExpType<uint32_t>
  Json<uint32_t>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits<uint32_t>(value.read_u64());
  }
  DLL_PUBLIC ExpType<uint32_t>
  Json<uint32_t>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits<uint32_t>(test_u64(value));
  }

  DLL_PUBLIC ExpType<int64_t>
  Json<int64_t>::deserialize(const JDt::JsonValue& value) {
    return test_i64(value);
  }

  DLL_PUBLIC ExpType<uint64_t>
  Json<uint64_t>::deserialize(const JDt::JsonValue& value) {
    return test_u64(value);
  }

  DLL_PUBLIC ExpType<float>
  Json<float>::deserialize(const JRd::JsonValue& value) {
    return test_numerical_limits(value.read_double());
  }
  DLL_PUBLIC ExpType<float>
  Json<float>::deserialize(const JDt::JsonValue& value) {
    return test_numerical_limits(test_double(value));
  }

  DLL_PUBLIC ExpType<double>
  Json<double>::deserialize(const JDt::JsonValue& value) {
    return test_double(value);
  }

  DLL_PUBLIC ExpType<std::string>
  Json<std::string>::deserialize(const Data::JsonValue& value) {
    return optional_to_exp_type(value.read_str(), JsonErrorTypes::Invalid,
                                "Not a std::string"sv)
        .transform([](auto v) {
          return std::string(v);
        });
  }

} // namespace JsonTypedefCodeGen::Deserialize

#endif
