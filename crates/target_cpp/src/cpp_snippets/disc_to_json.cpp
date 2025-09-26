

  template<> struct Serialize<$FULL_NAME$> {
      static ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$& value) {
        using Disc = $FULL_NAME$;
        using Types = Disc::Types;

        SHORT_EXP(serializer.start_object());
        std::string_view tag_name = Common<Disc>::entries[size_t(value.type())];
        SHORT_KEY_VAL("$TAG_KEY$"sv, tag_name);

        switch(value.type()) {
        default:
    $CLAUSES$        }
        return serializer.end_object();
    }
  };
