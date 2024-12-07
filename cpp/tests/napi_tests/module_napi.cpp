#include "utils.hpp"

#include <format>
#include <iostream>
#include <vector>

// ---------------------------------------------
namespace {

  enum class Status : int {
    Ok = 0,
    Failed
  };

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

    Status execute(Napi::Env env) {
      Status success = Status::Ok;
      auto fiter = m_functions.begin();
      for (auto name : m_names) {
        std::cout << std::format("\x1B[32mExecute: {}\x1B[0m\n", name);

        bool result = true;
        (*fiter)(env, result);
        if (result) {
          std::cout << "\x1B[32m---- OK\x1B[0m\n";
        } else {
          std::cout << "\x1B[31m---- FAILED\x1B[0m\n";
          success = Status::Failed;
        }
        ++fiter;
      }
      return success;
    }
  };

  static Runner test_runner;

} // namespace

TestItem::TestItem(const std::string_view name, TestFunc func) {
  test_runner.add(name, func);
}

// ---------------------------------------------
static Napi::Value run(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, int(test_runner.execute(env)));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "run"), Napi::Function::New(env, run));
  return exports;
}

NODE_API_MODULE(addon_napi, Init)
