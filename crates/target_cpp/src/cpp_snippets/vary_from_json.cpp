
  template<>
  struct Json<$FULL_NAME$> {
    using Vary = $FULL_NAME$;
    static constexpr std::string_view vary_name = "$VARY_NAME$"sv;
    $ENTRIES$
    $MANDATORY$

    static ExpType<Vary> deserialize(const Data::JsonObject& value) {
      $VISITED$
      Vary result;

      auto feach = json_object_for_each(
        value,
        [&](const std::string_view key, const auto val) {
          return flatten_expected(
            get_value_index(key, entries, vary_name)
            .transform([&](int idx) -> ExpType<void> {
                if (visited[idx]) {
                  return Errors::duplicated_key(key);
                }
                visited[idx] = true;

                switch (idx) {
                  default:// discriminator
                  case 0: return ExpType<void>();$CLAUSES$
                }
            }));
        });

      return chain_void_expected(
        feach,
        visited_mandatory(mandatory_indices, visited, entries, vary_name)
      ).transform([&result]() { return std::move(result); });
    }
  };
