
  ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$& value) {
    using Disc = $FULL_NAME$;
    using Types = Disc::Types;

    SHORT_EXP(serializer.start_object());
    SHORT_KEY_VAL("$TAG_KEY$"sv, Common<Disc>::entries[size_t(value.type())]);

    switch(value.type()) {
    default:
$CLAUSES$    }
    return serializer.end_object();
  }
