
  template<>
  struct Json<$FULL_NAME$> {
    using Struct = $FULL_NAME$;
    static constexpr std::string_view st_name = "$STRUCT_NAME$"sv;
    $MANDATORY$

    template<typename JValue>
    static ExpType<Struct> deserialize(const JValue& value) {
      $VISITED$
      Struct result;

      auto feach = json_object_for_each(
        value,
        [&](const auto key, const auto &val) {
          return flatten_expected(
            get_value_index(key, Common<Struct>::entries, st_name)
            .transform([&](const int idx) -> ExpType<void> {
              if (visited[idx]) {
                return Errors::duplicated_key(key);
              }
              visited[idx] = true;

              switch (idx) {
                default:$CLAUSES$
              }
            }));
        });

      return chain_void_expected(
        feach,
        visited_mandatory(mandatory_indices, visited, Common<Struct>::entries, st_name)
      ).transform([&result]() { return std::move(result); });
    }
  };
