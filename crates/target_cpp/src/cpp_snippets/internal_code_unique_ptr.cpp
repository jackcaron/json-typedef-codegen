
  template<typename Nullable>
  struct FromJson<std::unique_ptr<Nullable>> {
    using UniqueNull = std::unique_ptr<Nullable>;

    template<typename JValue>
    static ExpType<UniqueNull> convert(const JValue &value) {
      if (auto exp_null = value.is_null(); exp_null.has_value()) {
        if (exp_null.value()) {
          return ExpType<UniqueNull>(nullptr);
        }
      }
      else {
        return std::unexpected(std::move(exp_null.error()));
      }

      return FromJson<Nullable>::convert(value)
          .transform([](auto&& val) {
            return std::make_unique<Nullable>(std::move(val));
          });
    }
  };
