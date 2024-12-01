#pragma once

#include <functional>
#include <napi.h>
#include <string_view>

using TestFunc = std::function<void(Napi::Env, bool&)>;

// ---------------------------------------------

class TestItem {
private:
  bool m_ok = false;

public:
  TestItem() = delete;
  TestItem(const std::string_view name, TestFunc func);

  inline bool ok() const { return m_ok; }
};

// ---------------------------------------------
#define TEST_FULL_NAME(suite_name, test_name)                                  \
  std::string_view(#suite_name "." #test_name)

#define NAPI_TEST(suite_name, test_name)                                       \
  namespace suite_name::test_name {                                            \
    constexpr auto str = TEST_FULL_NAME(suite_name, test_name);                \
    void func(Napi::Env env, bool& success);                                   \
    static TestItem item = TestItem(str, func);                                \
  }                                                                            \
  void suite_name::test_name::func(Napi::Env env, bool& ___success)

#define NAPI_EXP_TRUE(expr)                                                    \
  if (const bool _res = (expr); !_res) {                                       \
    std::cout << "\t- expected " #expr " to be true\n";                        \
    ___success = false;                                                        \
    return;                                                                    \
  }

#define NAPI_EXP_FALSE(expr)                                                   \
  if (const bool _res = (expr); _res) {                                        \
    std::cout << "\t- expected " #expr " to be false\n";                       \
    ___success = false;                                                        \
    return;                                                                    \
  }

// ---------------------------------------------
