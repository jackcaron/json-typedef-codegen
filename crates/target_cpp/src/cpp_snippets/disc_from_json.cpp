
  template<>
  struct FromJson<$FULL_NAME$> {
    using Disc = $FULL_NAME$;
    static constexpr std::string_view discName = "$DISC_NAME$"sv;
    $ENTRIES$

    static ExpType<int> getDiscIndex(const Data::JsonObject& object) {
      auto exp_disc = getDiscValue(object, "$TAG_KEY$"sv, discName);

      return flatten_expected(
          exp_disc.transform([](const std::string_view disc) {
            return getValueIndex(disc, entries, discName);
          }));
    }

    static ExpType<Disc> toDisc(const Data::JsonObject& object, int idx) {
      constexpr auto cast = [](auto v) { return Disc(v); };
      switch (idx) {
        default:$CLAUSES$
      }
    }

    static ExpType<Disc> convert(const Data::JsonValue &value) {
      auto exp_obj = optional_to_exp_type(value.read_object(), JsonErrorTypes::Invalid, "not an object"sv);

      return flatten_expected(
          exp_obj.transform(
            [](const Data::JsonObject object) -> ExpType<Disc> {
              return flatten_expected(
                  getDiscIndex(object).transform([&](int idx) {
                    return toDisc(object, idx);
                  }));
            }));
    }

    static ExpType<Disc> convert(const Reader::JsonValue &value) {
      return flatten_expected(value.clone().transform([](Data::JsonValue val) {
        return FromJson<Disc>::convert(val);
      }));
    }
  };
