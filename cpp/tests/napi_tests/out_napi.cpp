#ifdef USE_OUT_NAPI

#include "napi.hpp"
#include "utils.hpp"

#include <iostream>
#include <type_traits>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

void __expect_invalid_serial(Napi::Env env, bool& ___success,
                             Napi::Value& root) {
  auto exp = Writer::napi_serializer(root);
  NAPI_EXP_FALSE(exp.has_value());
}
#define expect_invalid_serial(root)                                            \
  __expect_invalid_serial(env, ___success, root);                              \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_invalid_op(Napi::Env env, bool& ___success, ExpType<void> res) {
  NAPI_EXP_FALSE(res.has_value());

  auto err = std::move(res.error());
  NAPI_EXP_TRUE(err.type == JsonErrorTypes::Invalid);
}
#define expect_invalid_op(res)                                                 \
  __expect_invalid_op(env, ___success, res);                                   \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_ok_op(Napi::Env env, bool& ___success, ExpType<void> res) {
  NAPI_EXP_TRUE(res.has_value());
}
#define expect_ok_op(res)                                                      \
  __expect_ok_op(env, ___success, res);                                        \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_serial(Napi::Env env, bool& ___success, Napi::Value& root,
                     std::function<void(Writer::Serializer&)> fn,
                     bool exp_complete = true) {
  auto exp = Writer::napi_serializer(root);
  NAPI_EXP_TRUE(exp.has_value());

  auto serial = std::move(exp.value());
  fn(serial);

  if (exp_complete) {
    expect_ok_op(serial.close());
  } else {
    expect_invalid_op(serial.close());
  }
}
#define expect_serial(r, ...)                                                  \
  __expect_serial(env, ___success, r, __VA_ARGS__);                            \
  if (!___success) {                                                           \
    return;                                                                    \
  }

template <typename Num>
void __expect_number(Napi::Env env, bool& ___success, Napi::Value val,
                     const Num exp) {
  if constexpr (std::is_same_v<Num, double>) {
    NAPI_EXP_TRUE(val.IsNumber());
    auto nb = val.ToNumber();
    NAPI_EXP_TRUE(nb.DoubleValue() == exp);
  } else {
    NAPI_EXP_TRUE(val.IsBigInt());
    auto nb = val.As<Napi::BigInt>();
    bool loss = false;
    if constexpr (std::is_same_v<Num, uint64_t>) {
      NAPI_EXP_TRUE(nb.Uint64Value(&loss) == exp);
    } else {
      NAPI_EXP_TRUE(nb.Int64Value(&loss) == exp);
    }
  }
}
#define expect_number(v, x)                                                    \
  __expect_number(env, ___success, v, x);                                      \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_str(Napi::Env env, bool& ___success, Napi::Value val,
                  const std::string_view exp) {
  NAPI_EXP_TRUE(val.IsString());
  auto nxp = Napi::String::New(env, exp.data(), exp.size());
  NAPI_EXP_TRUE(val.ToString() == nxp);
}
#define expect_str(v, x)                                                       \
  __expect_str(env, ___success, v, x);                                         \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_bool(Napi::Env env, bool& ___success, Napi::Value val,
                   const bool exp) {
  NAPI_EXP_TRUE(val.IsBoolean());
  NAPI_EXP_TRUE(val.ToBoolean() == exp);
}
#define expect_bool(v, x)                                                      \
  __expect_bool(env, ___success, v, x);                                        \
  if (!___success) {                                                           \
    return;                                                                    \
  }

void __expect_obj_size(Napi::Env env, bool& ___success, Napi::Object& obj,
                       const uint32_t exp) {
  NAPI_EXP_TRUE(obj.GetPropertyNames().Length() == exp);
}
#define expect_obj_size(v, x)                                                  \
  __expect_obj_size(env, ___success, v, x);                                    \
  if (!___success) {                                                           \
    return;                                                                    \
  }

// -----------------------------------------

NAPI_TEST(NAPI_OUT, invalid_root) {
  {
    auto js = env.Null();
    expect_invalid_serial(js);
  }
  {
    auto js = Napi::Boolean::New(env, false);
    expect_invalid_serial(js);
  }
  {
    auto js = Napi::String::New(env, "bob");
    expect_invalid_serial(js);
  }
  {
    auto js = Napi::Number::New(env, 23.0);
    expect_invalid_serial(js);
  }
}

NAPI_TEST(NAPI_OUT, invalid_arrays_ops) {
  auto jsarr = Napi::Array::New(env);
  expect_serial(jsarr, [&](auto& serial) {
    expect_invalid_op(serial.end_array());
    expect_invalid_op(serial.end_object());
  });
}

NAPI_TEST(NAPI_OUT, invalid_object_ops) {
  auto jsobj = Napi::Object::New(env);
  expect_serial(jsobj, [&](auto& serial) {
    expect_invalid_op(serial.end_array());
    expect_invalid_op(serial.start_object());
    expect_invalid_op(serial.end_object());
    expect_invalid_op(serial.write_bool(false));
  });
}

NAPI_TEST(NAPI_OUT, primitive_arrays) {
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.write_bool(true));
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
    NAPI_EXP_TRUE(jsarr.Get(0u).IsBoolean());
    NAPI_EXP_TRUE(jsarr.Get(0u).ToBoolean());
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.write_i64(1));
      expect_ok_op(serial.write_i64(2));
    });

    NAPI_EXP_TRUE(jsarr.Length() == 2);
    expect_number(jsarr.Get(0u), 1);
    expect_number(jsarr.Get(1u), 2);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.write_double(1.0));
      expect_ok_op(serial.write_str("bob"sv));
      expect_ok_op(serial.write_bool(false));
    });

    NAPI_EXP_TRUE(jsarr.Length() == 3);
    expect_number(jsarr.Get(0u), 1.0);
    expect_str(jsarr.Get(1u), "bob"sv);
    expect_bool(jsarr.Get(2u), false);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.write_null());
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
    NAPI_EXP_TRUE(jsarr.Get(0u).IsNull());
  }
}

NAPI_TEST(NAPI_OUT, primitive_objects) {
  {
    auto jsobj = Napi::Object::New(env);
    expect_serial(jsobj, [&](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(true));
    });

    expect_obj_size(jsobj, 1);
    expect_bool(jsobj.Get("bob"), true);
  }
  {
    auto jsobj = Napi::Object::New(env);
    expect_serial(jsobj, [&](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_u64(29));

      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_u64(32));
    });

    expect_obj_size(jsobj, 2);
    expect_number(jsobj.Get("bob"), 29ul);
    expect_number(jsobj.Get("alice"), 32ul);
  }
  {
    auto jsobj = Napi::Object::New(env);
    expect_serial(jsobj, [&](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_u64(29));

      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_null());

      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(false));
    });

    expect_obj_size(jsobj, 2);
    expect_bool(jsobj.Get("bob"), false);
    NAPI_EXP_TRUE(jsobj.Get("alice").IsNull());
  }
}

NAPI_TEST(NAPI_OUT, array_of_items) {
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.end_object());
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(false));
      expect_ok_op(serial.end_object());
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
    NAPI_EXP_TRUE(jsarr.Get(0u).IsObject());
    auto jsobj = jsarr.Get(0u).ToObject();
    expect_obj_size(jsobj, 1);
    expect_bool(jsobj.Get("bob"), false);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      for (int i = 0; i < 4; ++i) {
        expect_ok_op(serial.start_object());
        expect_ok_op(serial.end_object());
      }
    });
    NAPI_EXP_TRUE(jsarr.Length() == 4);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.start_array());
      expect_ok_op(serial.write_bool(true));
      expect_ok_op(serial.end_array());
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
    NAPI_EXP_TRUE(jsarr.Get(0u).IsArray());
    auto subarr = jsarr.Get(0u).As<Napi::Array>();
    NAPI_EXP_TRUE(subarr.Length() == 1);
    expect_bool(subarr.Get(0u), true);
  }
  {
    auto jsarr = Napi::Array::New(env);
    expect_serial(jsarr, [&](auto& serial) {
      expect_ok_op(serial.start_array());
      for (int i = 0; i < 20; ++i) {
        expect_ok_op(serial.write_u64(i));
      }
      expect_ok_op(serial.end_array());
    });

    NAPI_EXP_TRUE(jsarr.Length() == 1);
    NAPI_EXP_TRUE(jsarr.Get(0u).IsArray());
    auto subarr = jsarr.Get(0u).As<Napi::Array>();
    NAPI_EXP_TRUE(subarr.Length() == 20);
    for (uint64_t i = 0; i < 20; ++i) {
      expect_number(subarr.Get(i), i);
    }
  }
}

NAPI_TEST(NAPI_OUT, object_of_items) {
  {
    auto jsobj = Napi::Object::New(env);
    expect_serial(jsobj, [&](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.start_array());
      for (int i = 0; i < 20; ++i) {
        expect_ok_op(serial.write_u64(i));
      }
      expect_ok_op(serial.end_array());
    });

    expect_obj_size(jsobj, 1);
    NAPI_EXP_TRUE(jsobj.Get("bob").IsArray());
    auto subarr = jsobj.Get("bob").As<Napi::Array>();
    NAPI_EXP_TRUE(subarr.Length() == 20);
    for (uint64_t i = 0; i < 20; ++i) {
      expect_number(subarr.Get(i), i);
    }
  }
  {
    auto jsobj = Napi::Object::New(env);
    expect_serial(jsobj, [&](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_bool(false));
      expect_ok_op(serial.end_object());
    });

    expect_obj_size(jsobj, 1);
    NAPI_EXP_TRUE(jsobj.Get("bob").IsObject());
    auto subobj = jsobj.Get("bob").ToObject();
    expect_bool(subobj.Get("alice"), false);
  }
  {
    // Closing an object with a pending key
    auto jsobj = Napi::Object::New(env);
    expect_serial(
        jsobj,
        [&](auto& serial) {
          expect_ok_op(serial.write_key("bob"sv));
          expect_ok_op(serial.start_object());
          expect_ok_op(serial.write_key("alice"sv));
          expect_invalid_op(serial.end_object());
        },
        false);
  }
}

#endif
