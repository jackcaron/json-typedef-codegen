
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
      JsonMap<Type> result;
      auto feach =
        json_object_for_each(value, [&](const auto key, const auto &val) -> ExpType<void> {
          if (auto exp_res = FromJson<Type>::convert(val); exp_res.has_value()) {
            auto [_iter, ok] = result.insert({ std::string(key), exp_res.value() });
            if (ok) {
              return ExpType<void>();
            }
            else {
              const auto err = std::format("Duplicated key \"{}\"", key);
              return makeJsonError(JsonErrorTypes::String, err);
            }
          }
          else {
            return makeJsonError(exp_res.error());
          }
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };

  ExpType<void> visited_mandatory(
    const std::span<const int> mandatory_indices,
    const std::span<bool> visited,
    const std::span<const std::string_view> entries,
    const std::string_view name) {
    //
    for (const auto midx : mandatory_indices) {
      if (!visited[midx]) {
        const auto err = std::format("Missing key \"{}\" for {}", entries[midx], name);
        return makeJsonError(JsonErrorTypes::String, err);
      }
    }
    return ExpType<void>();
  }
