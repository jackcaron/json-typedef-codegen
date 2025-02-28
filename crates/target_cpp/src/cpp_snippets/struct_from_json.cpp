
  template<>
  struct FromJson<$FULL_NAME$> {
    using Struct = $FULL_NAME$;
    static constexpr std::string_view st_name = "$STRUCT_NAME$"sv;
    $ENTRIES$
    $MANDATORY$

    template<typename JValue>
    static ExpType<Struct> convert(const JValue& value) {
      $VISITED$
      Struct result;

      auto feach = json_object_for_each(
        value,
        [&](const auto key, const auto &val) {
          return flatten_expected(
            getValueIndex(key, entries, st_name)
            .transform([&](const int idx) -> ExpType<void> {
              if (visited[idx]) {
                const auto err = std::format("Duplicated key {}", key);
                return makeJsonError(JsonErrorTypes::Invalid, err);
              }
              visited[idx] = true;

              switch (idx) {
                default:$CLAUSES$
              }
            }));
        });

      return chain_void_expected(
        feach,
        visited_mandatory(mandatory_indices, visited, entries, st_name)
      ).transform([res = std::move(result)]() { return res; });
    }
  };
