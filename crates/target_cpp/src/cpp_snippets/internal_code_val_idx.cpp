

  ExpType<std::string> get_disc_value(const Data::JsonObject& object,
                                      const std::string_view disc,
                                      const std::string_view name) {
    auto& inner = object.internal();
    const auto fnd = inner.find(std::string(disc));
    if (fnd == inner.end()) {
      auto err = format("missing key \"{}\" for {}", disc, name);
      return make_json_error(JsonErrorTypes::Invalid, err);
    }

    const auto opt_str = fnd->second.read_str();
    if (opt_str.has_value()) {
      return std::string(opt_str.value());
    }

    const auto err = format("expected string value for {}", disc);
    return make_json_error(JsonErrorTypes::Invalid, err);
  }

  constexpr ExpType<int> get_value_index(const std::string_view value,
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
    return make_json_error(JsonErrorTypes::Invalid, err);
  }
