
  template <typename JValue, typename Cb>
  auto json_array_for_each(const JValue &value, Cb cb) {
    if constexpr (std::is_same_v<JValue, Reader::JsonValue>) {
      return Reader::json_array_for_each(value, cb);
    } else if constexpr (std::is_same_v<JValue, Data::JsonValue>) {
      return Data::json_array_for_each(value, cb);
    }
  }

  template<typename Type>
  struct FromJson<std::vector<Type>> {
    template<typename JValue>
    static ExpType<std::vector<Type>> convert(const JValue& value) {
      auto converter = FromJson<Type>::convert;
      std::vector<Type> result;
      auto feach =
        json_array_for_each(value, [&](const auto &item) {
          if (auto exp_res = converter(item); exp_res.has_value()) {
            result.emplace_back(std::move(exp_res.value()));
            return ExpType<void>();
          } else {
            return UnexpJsonError(exp_res.error());
          }
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };
