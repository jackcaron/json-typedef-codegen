
  template <typename JValue, typename Cb>
  auto json_object_for_each(const JValue &value, Cb cb) {
    if constexpr (std::is_same_v<JValue, Reader::JsonValue>) {
      return Reader::json_object_for_each(value, cb);
    } else if constexpr (std::is_same_v<JValue, Data::JsonValue>) {
      return Data::json_object_for_each(value, cb);
    }
  }

  template<typename Type>
  struct FromJson<JsonMap<Type>> {
    template <typename JValue>
    static ExpType<JsonMap<Type>> convert(const JValue& value) {
      auto converter = FromJson<Type>::convert;
      JsonMap<Type> result;
      auto feach =
        json_object_for_each(value, [&](const auto key, const auto &val) {
          if (auto exp_res = converter(val); exp_res.has_value()) {
            auto [_iter, ok] = result.insert({ std::string(key), exp_res.value() });
            if (ok) {
              return ExpType<void>();
            }
            else {
              const auto err = std::format("Duplicated key \"{}\"", key);
              return UnexpJsonError(JsonErrorTypes::String, err);
            }
          }
          else {
            return UnexpJsonError(exp_res.error());
          }
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };
