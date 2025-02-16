
  template<>
  struct FromJson<Data::JsonValue> {
    static ExpType<Data::JsonValue> convert(const Reader::JsonValue& v) {
      return v.clone();
    }
    static ExpType<Data::JsonValue> convert(const Data::JsonValue& v) {
      return v;
    }
  };
