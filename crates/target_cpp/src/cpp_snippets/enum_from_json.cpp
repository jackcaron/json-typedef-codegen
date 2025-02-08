
  template<>
  struct FromJson<FULL_NAME> {
    ENTRIES

    template<typename JValue>
    static ExpType<FULL_NAME> convert(const JValue &value) {
      using FULL_NAME;
      return getEnumIndex(value, entries, "ENUM_NAME"sv)
        .transform([](auto idx) {
          switch(idx) {
            default:
CLAUSES        }
      });
    }
  };
