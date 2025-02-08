
  template<>
  struct FromJson<FULL_NAME> {
    using Disc = FULL_NAME;
    ENTRIES

    static ExpType<Disc> convert(const Data::JsonValue &value) {
      auto exp_obj = optionalToExpType(value.read_object(), JsonErrorTypes::Invalid, "not an object"sv);

      return flatten_expected(
        exp_obj.transform([](Data::JsonObject object) {
          auto& inner = object.internal();
          const auto fnd = inner.find(std::string("TAG_KEY"sv));
          if (fnd == inner.end()) {
            return makeJsonError(JsonErrorTypes::Invalid, "missing key \"TAG_KEY\" for DISC_NAME"sv);
          }

          return getValueIndex(fnd->second, entries, "DISC_NAME"sv)
            .transform([&object](int idx) {
              constexpr auto toDisc = [](auto v) { return Disc(v); };
              switch (idx) {
                default:CLAUSES
              }
            });
        }));
    }
    static ExpType<Disc> convert(const Reader::JsonValue &value) {
      return flatten_expected(value.clone().transform(FromJson<Disc>::convert));
    }
  };
