
  template<>
  struct Json<$FULL_NAME$> {
    using Enum = $FULL_NAME$;

    template<typename JValue>
    static ExpType<Enum> deserialize(const JValue &value) {
      return get_enum_index(value, Common<Enum>::entries, "$ENUM_NAME$"sv)
        .transform([](int idx) { return (Enum)idx; });
    }
  };
