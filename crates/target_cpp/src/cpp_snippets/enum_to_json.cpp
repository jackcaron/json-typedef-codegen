
  ExpType<void> serialize(Writer::Serializer& serializer, const $FULL_NAME$ value) {
    using Enum = $FULL_NAME$;
    return serializer.write_str(Common<Enum>::entries[size_t(value)]);
  }
