namespace {

  using namespace JsonTypedefCodeGen;
  using namespace std::string_view_literals;

  template<typename Type> struct FromJson;

  template <typename Type>
  ExpType<void> convert_and_set(Type &dst, const Reader::JsonValue &value) {
    return FromJson<Type>::convert(value).transform([&dst](auto v) { dst = std::move(v); });
  }

  template <typename Type>
  ExpType<void> convert_and_set(Type &dst, const Data::JsonValue &value) {
    return FromJson<Type>::convert(value).transform([&dst](auto v) { dst = std::move(v); });
  }

  template <typename Type>
  ExpType<Type> optional_to_exp_type(const std::optional<Type> &opt,
                                     const JsonErrorTypes errtype,
                                     const std::string_view msg) {
    if (opt.has_value()) {
      return *opt;
    }
    return makeJsonError(errtype, msg);
  }
