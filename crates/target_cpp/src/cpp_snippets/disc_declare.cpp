
class DISCRIMINATOR_NAME {
private:
  using variant_t = std::variant<VARIANT_NAMES>;
  variant_t m_value;

public:
  enum class Types: size_t {
    TYPE_VALUES
  };

  constexpr DISCRIMINATOR_NAME() = default;
  template<typename U> constexpr DISCRIMINATOR_NAME(U& t): m_value(t) {}

  template<typename U> constexpr DISCRIMINATOR_NAME& operator=(U& t) {
    m_value = t;
    return *this;
  }

  constexpr Types type() const { return Types(m_value.index()); }

  template<Types Tp> constexpr auto get() {
    namespace DT = JsonTypedefCodeGen::Data;
    if (auto ptr = std::get_if<size_t(Tp)>(&m_value); ptr != nullptr) {
      return DT::OptRefW(DT::RefW(*ptr));
    }
    return DT::OptRefW();
  }

  template<Types Tp> constexpr auto get() const {
    namespace DT = JsonTypedefCodeGen::Data;
    if (auto ptr = std::get_if<size_t(Tp)>(&m_value); ptr != nullptr) {
      return DT::OptRefW(DT::RefW(*ptr));
    }
    return DT::OptRefW();
  }
};
