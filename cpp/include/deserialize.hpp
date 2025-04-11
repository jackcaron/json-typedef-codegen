#pragma once

#include "json_data.hpp"
#include "json_reader.hpp"

#include <format>
#include <memory>
#include <span>

// utility functions for the deserialized generated code

namespace JsonTypedefCodeGen::Deserialize {

  namespace JDt = JsonTypedefCodeGen::Data;
  namespace JRd = JsonTypedefCodeGen::Reader;

  template <typename Type> struct Json;

  //  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

  template <typename Type>
  ExpType<void> deserialize_and_set(Type& dst, const JRd::JsonValue& value) {
    return Json<Type>::deserialize(value).transform([&dst](auto v) {
      dst = std::move(v);
    });
  }

  template <typename Type>
  ExpType<void> deserialize_and_set(Type& dst, const JDt::JsonValue& value) {
    return Json<Type>::deserialize(value).transform([&dst](auto v) {
      dst = std::move(v);
    });
  }

  template <typename Type>
  ExpType<Type> optional_to_exp_type(const std::optional<Type>& opt,
                                     const JsonErrorTypes errtype,
                                     const std::string_view msg) {
    if (opt.has_value()) {
      return *opt;
    }
    return make_json_error(errtype, msg);
  }

  template <typename JValue>
  ExpType<int> get_enum_index(const JValue& value,
                              const std::span<const std::string_view> entries,
                              const std::string_view enumName) {
    using namespace std::string_view_literals;
    if (const auto str = value.read_str(); str.has_value()) {
      const auto val = std::move(str.value());
      for (int index = 0; const auto entry : entries) {
        if (val == entry) {
          return index;
        }
        ++index;
      }
      const auto err =
          std::format("Invalid value \"{}\" for {}"sv, val, enumName);
      return make_json_error(JsonErrorTypes::Invalid, err);
    } else {
      if constexpr (std::is_same_v<JValue, JDt::JsonValue>) {
        const auto err = std::format("Not a string for {}"sv, enumName);
        return make_json_error(JsonErrorTypes::Invalid, err);
      } else {
        return std::unexpected(str.error());
      }
    }
  }

  template <typename JValue, typename Cb>
  constexpr ExpType<void> json_object_for_each(const JValue& value, Cb cb) {
    if constexpr (std::is_same_v<JValue, JRd::JsonValue> ||
                  std::is_same_v<JValue, JRd::JsonObject>) {
      return JRd::json_object_for_each(value, cb);
    } else {
      // std::is_same_v<JValue, JDt::JsonValue>
      return JDt::json_object_for_each(value, cb);
    }
  }

  template <typename JValue, typename Cb>
  constexpr ExpType<void> json_array_for_each(const JValue& value, Cb cb) {
    if constexpr (std::is_same_v<JValue, JRd::JsonValue> ||
                  std::is_same_v<JValue, JRd::JsonArray>) {
      return JRd::json_array_for_each(value, cb);
    } else {
      // std::is_same_v<JValue, JDt::JsonValue>
      return JDt::json_array_for_each(value, cb);
    }
  }

  //  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
  ExpType<void>
  visited_mandatory(const std::span<const int> mandatory_indices,
                    const std::span<bool> visited,
                    const std::span<const std::string_view> entries,
                    const std::string_view name);

  ExpType<std::string> get_disc_value(const JDt::JsonObject& object,
                                      const std::string_view disc,
                                      const std::string_view name);

  ExpType<int> get_value_index(const std::string_view value,
                               const std::span<const std::string_view> entries,
                               const std::string_view structName);

  //  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

  // more primitives

  template <> struct Json<bool> {
    static ExpType<bool> deserialize(const JRd::JsonValue& value);
    static ExpType<bool> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<int8_t> {
    static ExpType<int8_t> deserialize(const JRd::JsonValue& value);
    static ExpType<int8_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<uint8_t> {
    static ExpType<uint8_t> deserialize(const JRd::JsonValue& value);
    static ExpType<uint8_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<int16_t> {
    static ExpType<int16_t> deserialize(const JRd::JsonValue& value);
    static ExpType<int16_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<uint16_t> {
    static ExpType<uint16_t> deserialize(const JRd::JsonValue& value);
    static ExpType<uint16_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<int32_t> {
    static ExpType<int32_t> deserialize(const JRd::JsonValue& value);
    static ExpType<int32_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<uint32_t> {
    static ExpType<uint32_t> deserialize(const JRd::JsonValue& value);
    static ExpType<uint32_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<int64_t> {
    static ExpType<int64_t> deserialize(const JRd::JsonValue& value);
    static ExpType<int64_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<uint64_t> {
    static ExpType<uint64_t> deserialize(const JRd::JsonValue& value);
    static ExpType<uint64_t> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<float> {
    static ExpType<float> deserialize(const JRd::JsonValue& value);
    static ExpType<float> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<double> {
    static ExpType<double> deserialize(const JRd::JsonValue& value);
    static ExpType<double> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<std::string> {
    static ExpType<std::string> deserialize(const JRd::JsonValue& value);
    static ExpType<std::string> deserialize(const JDt::JsonValue& value);
  };

  template <> struct Json<Data::JsonValue> {
    static ExpType<Data::JsonValue> deserialize(const Reader::JsonValue& v);
    static ExpType<Data::JsonValue> deserialize(const Data::JsonValue& v);
  };

  template <typename Type> struct Json<std::vector<Type>> {
    template <typename JValue>
    static ExpType<std::vector<Type>> deserialize(const JValue& value) {
      std::vector<Type> result;
      auto feach =
          json_array_for_each(value, [&](const auto& item) -> ExpType<void> {
            if (auto exp_res = Json<Type>::deserialize(item);
                exp_res.has_value()) {
              result.emplace_back(std::move(exp_res.value()));
              return ExpType<void>();
            } else {
              return UnexpJsonError(exp_res.error());
            }
          });
      return feach.transform([res = std::move(result)]() {
        return res;
      });
    }
  };

#ifdef __JTD_USE_UNORDERED_MAP
  template <typename Type>
  using JsonMap = std::unordered_map<std::string, Type>;
#else
  template <typename Type> using JsonMap = std::map<std::string, Type>;
#endif

  template <typename Type> struct Json<JsonMap<Type>> {
    template <typename JValue>
    static ExpType<JsonMap<Type>> deserialize(const JValue& value) {
      JsonMap<Type> result;
      auto feach = json_object_for_each(
          value, [&](const auto key, const auto& val) -> ExpType<void> {
            if (auto exp_res = Json<Type>::deserialize(val);
                exp_res.has_value()) {

              if (result.insert({std::string(key), exp_res.value()}).second) {
                return ExpType<void>();
              } else {
                using namespace std::string_view_literals;
                const auto err = std::format("Duplicated key \"{}\""sv, key);
                return make_json_error(JsonErrorTypes::String, err);
              }
            } else {
              return make_json_error(exp_res.error());
            }
          });
      return feach.transform([res = std::move(result)]() {
        return res;
      });
    }
  };

  template <typename Nullable> struct Json<std::unique_ptr<Nullable>> {
    using UniqueNull = std::unique_ptr<Nullable>;

    template <typename JValue>
    static ExpType<UniqueNull> deserialize(const JValue& value) {
      if (auto exp_null = value.is_null(); exp_null.has_value()) {
        if (exp_null.value()) {
          return ExpType<UniqueNull>(nullptr);
        }
      } else {
        return std::unexpected(std::move(exp_null.error()));
      }

      return Json<Nullable>::deserialize(value).transform([](auto&& val) {
        return std::make_unique<Nullable>(std::move(val));
      });
    }
  };

} // namespace JsonTypedefCodeGen::Deserialize
