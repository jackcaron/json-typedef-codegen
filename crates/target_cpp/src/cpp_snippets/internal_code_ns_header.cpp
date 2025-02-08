namespace {

  using namespace JsonTypedefCodeGen;
  using namespace std::string_view_literals;

  template<typename Type> struct FromJson;

  template <typename Type>
  ExpType<void> convertAndSet(Type &dst, const Reader::JsonValue &value) {
    return FromJson<Type>::convert(value).transform([&dst](auto v) { dst = v; });
  }

  template <typename Type>
  ExpType<void> convertAndSet(Type &dst, const Data::JsonValue &value) {
    return FromJson<Type>::convert(value).transform([&dst](auto v) { dst = v; });
  }

  template <typename Type>
  ExpType<Type> optionalToExpType(const std::optional<Type> &opt,
                                  const JsonErrorTypes errtype,
                                  const std::string_view msg) {
    if (opt.has_value()) {
      return *opt;
    }
    return makeJsonError(errtype, msg);
  }
