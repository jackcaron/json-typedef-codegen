

  template<> struct Serialize<$FULL_NAME$> {
      static ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$& value) {
        using Disc = $FULL_NAME$;
        using Types = Disc::Types;

        SHORT_EXP(serializer.start_object());
        const std::string_view tag_name = Common<Disc>::entries[size_t(value.type())];
        SHORT_EXP(serialize_key_value(serializer, "$TAG_KEY$"sv, tag_name));

        switch(value.type()) {
        default:
    $CLAUSES$        }
        return serializer.end_object();
    }
  };
