
  constexpr ExpType<int> getValueIndex(const std::string_view value,
                                       const std::span<const std::string_view> entries,
                                       const std::string_view structName) {
    for (int index = 0; const auto entry : entries) {
      if (value == entry) {
        return index;
      }
      ++index;
    }
    const auto err = std::format("Invalid key \"{}\" in {}",
                                 value, structName);
    return makeJsonError(JsonErrorTypes::Invalid, err);
  }
