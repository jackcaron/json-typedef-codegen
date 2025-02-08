
  template<>
  struct FromJson<FULL_NAME> {
    using Struct = FULL_NAME;
    ENTRIES

    template<typename JValue>
    static ExpType<Struct> convert(const JValue &value) {
      VISITED
      Struct result;

      auto feach = json_object_for_each(
        value,
        [&](const auto key, const auto &val) {
          return flatten_expected(
            getValueIndex(key, entries, "STRUCT_NAME"sv)
            .transform([&](const int idx) -> ExpType<void> {
              if (visited[idx]) {
                const auto err = std::format("Duplicated key {}", key);
                return makeJsonError(JsonErrorTypes::Invalid, err);
              }
              visited[idx] = true;

              switch (idx) {
                default:CLAUSES
              }
            }));
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };
