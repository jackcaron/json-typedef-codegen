
  template<>
  struct FromJson<$FULL_NAME$> {
    using Enum = $FULL_NAME$;
    $ENTRIES$

    template<typename JValue>
    static ExpType<Enum> convert(const JValue &value) {
      return getEnumIndex(value, entries, "$ENUM_NAME$"sv)
        .transform([](auto idx) {
          switch(idx) {
            default:
$CLAUSES$        }
      });
    }
  };
