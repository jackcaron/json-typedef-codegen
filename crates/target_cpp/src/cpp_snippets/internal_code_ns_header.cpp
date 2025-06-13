using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

#define SHORT_EXP(expr)                                                        \
  if (ExpType<void> exp = (expr); !exp.has_value()) {                          \
    return exp;                                                                \
  }

#define SHORT_KEY_VAL(key, val)                                                \
  SHORT_EXP(serializer.write_key((key)));                                      \
  SHORT_EXP(serialize(serializer, (val)));
