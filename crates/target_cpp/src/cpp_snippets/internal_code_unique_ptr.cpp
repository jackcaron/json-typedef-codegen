
  template<typename Nullable>
  struct FromJson<std::unique_ptr<Nullable>> {
    using UniqueNull = std::unique_ptr<Nullable>;

    template<typename JValue>
    static ExpType<UniqueNull> convert(const JValue &value) {
      return FromJson<Nullable>::convert(value).transform([](auto&& val) {
        return std::make_unique<Nullable>(std::move(val));
      })
    }
  };
