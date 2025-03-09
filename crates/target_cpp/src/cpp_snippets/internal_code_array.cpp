
  template <typename JValue, typename Cb>
  constexpr ExpType<void> json_array_for_each(const JValue &value, Cb cb) {
    if constexpr (std::is_same_v<JValue, Reader::JsonValue> || std::is_same_v<JValue, Reader::JsonArray>) {
      return Reader::json_array_for_each(value, cb);
    }
    else {
      // std::is_same_v<JValue, Data::JsonValue>
      return Data::json_array_for_each(value, cb);
    }
  }

  template<typename Type>
  struct FromJson<std::vector<Type>> {
    template<typename JValue>
    static ExpType<std::vector<Type>> deserialize(const JValue& value) {
      std::vector<Type> result;
      auto feach =
        json_array_for_each(value, [&](const auto &item) -> ExpType<void> {
          if (auto exp_res = FromJson<Type>::deserialize(item); exp_res.has_value()) {
            result.emplace_back(std::move(exp_res.value()));
            return ExpType<void>();
          } else {
            return UnexpJsonError(exp_res.error());
          }
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };
