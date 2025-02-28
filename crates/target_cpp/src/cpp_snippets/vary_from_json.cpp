
  template<>
  struct FromJson<$FULL_NAME$> {
    using Vary = $FULL_NAME$;
    $ENTRIES$

    template<typename JValue>
    static ExpType<Vary> convert(const JValue& object) {
      Vary result;
      auto feach = json_object_for_each(object, [&result](auto key, auto val) {
          $VISITED$
          return getValueIndex(key, entries, "$VARY_NAME$"sv)
            .transform([&](int idx) -> ExpType<void> {
                if (!visited[idx]) {
                  const auto err = std::format("Duplicated key {}", key);
                  return makeJsonError(JsonErrorTypes::Invalid, err);
                }
                visited[idx] = true;

                switch (idx) {
                  default:// discriminator
                  case 0: break;$CLAUSES$
                }
            });
        });
      return feach.transform([res = std::move(result)]() { return res; });
    }
  };
