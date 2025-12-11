using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

#define SHORT_EXP(expr)                                                        \
  if (ExpType<void> exp = (expr); !exp.has_value()) {                          \
    return exp;                                                                \
  }
