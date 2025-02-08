
  template<typename JValue>
  ExpType<int> getEnumIndex(const JValue &value,
                            const std::span<const std::string_view> entries,
                            const std::string_view enumName) {
    if (const auto str = value.read_str(); str.has_value()) {
      const auto val = std::move(str.value());
      for (int index = 0; const auto entry : entries) {
        if (val == entry) {
         return index;
        }
        ++index;
      }
      const auto err = std::format("Invalid value \"{}\" for {}",
                                   value, enumName);
      return makeJsonError(JsonErrorTypes::Invalid, err);
    }
    else {
      if constexpr (std::is_same_v<JValue, Data::JsonValue>) {
        const auto err = std::format("Not a string for {}", enumName);
        return makeJsonError(JsonErrorTypes::Invalid, err);
      }
      else {
        return std::unexpected(str.error());
      }
    }
  }
