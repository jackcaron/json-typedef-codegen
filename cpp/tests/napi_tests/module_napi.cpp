#include "utils.hpp"

#include <iostream>
#include <vector>

// ---------------------------------------------
class Runner {
private:
  std::vector<std::string_view> m_names;
  std::vector<TestFunc> m_functions;

public:
  Runner() = default;

  bool add(const std::string_view name, TestFunc func) {
    m_names.emplace_back(name);
    m_functions.emplace_back(func);
    return true;
  }

  int execute(Napi::Env env) {
    // TODO: improve logging with colors
    int success = 0;
    std::cout << "Executing " << m_names.size() << " tests\n";
    auto fiter = m_functions.begin();
    for (auto name : m_names) {
      std::cout << "Execute: " << name << "\n";

      bool result = true;
      (*fiter)(env, result);
      if (result) {
        std::cout << "---- OK\n";
      } else {
        std::cout << "---- FAILED\n";
        success = 1;
      }
      ++fiter;
    }
    return success;
  }
};

static Runner test_runner;

TestItem::TestItem(const std::string_view name, TestFunc func)
    : m_ok(test_runner.add(name, func)) {}

// ---------------------------------------------
static Napi::Value run(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, test_runner.execute(env));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "run"), Napi::Function::New(env, run));
  return exports;
}

NODE_API_MODULE(addon_napi, Init)
