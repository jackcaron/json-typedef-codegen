
  template<>
  struct Json<$FULL_NAME$> {
    using Disc = $FULL_NAME$;
    static constexpr std::string_view discName = "$DISC_NAME$"sv;

    static ExpType<int> get_disc_index(const Data::JsonObject& object) {
      auto exp_disc = get_disc_value(object, "$TAG_KEY$"sv, discName);

      return flatten_expected(
          exp_disc.transform([](const std::string_view disc) {
            return get_value_index(disc, Common<Disc>::entries, discName);
          }));
    }

    static ExpType<Disc> to_disc(const Data::JsonObject& object, int idx) {
      constexpr auto cast = [](auto v) { return Disc(v); };
      switch (idx) {
        default:$CLAUSES$
      }
    }

    static ExpType<Disc> deserialize(const Data::JsonValue &value) {
      auto exp_obj = optional_to_exp_type(value.read_object(), JsonErrorTypes::Invalid, "not an object"sv);

      return flatten_expected(
          exp_obj.transform(
            [](const Data::JsonObject object) -> ExpType<Disc> {
              return flatten_expected(
                  get_disc_index(object).transform([&](int idx) {
                    return to_disc(object, idx);
                  }));
            }));
    }

    static ExpType<Disc> deserialize(const Reader::JsonValue &value) {
      return flatten_expected(value.clone().transform([](Data::JsonValue val) {
        return Json<Disc>::deserialize(val);
      }));
    }
  };
