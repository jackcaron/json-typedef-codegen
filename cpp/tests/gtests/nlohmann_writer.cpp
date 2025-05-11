
#ifdef USE_OUT_NLOH

#include "nlohmann.hpp"

#include <functional>
#include <gtest/gtest.h>

using namespace JsonTypedefCodeGen;
using namespace std::string_view_literals;

using NJson = nlohmann::json;

void expect_invalid_serial(NJson& js) {
  auto exp = Writer::nlohmann_serializer(js);
  EXPECT_FALSE(exp.has_value());

  auto err = std::move(exp.error());
  EXPECT_EQ(err.type, JsonErrorTypes::Invalid);
}

void expect_invalid_op(ExpType<void> res) {
  EXPECT_FALSE(res.has_value());

  auto err = std::move(res.error());
  EXPECT_EQ(err.type, JsonErrorTypes::Invalid);
}

void expect_ok_op(ExpType<void> res) { //
  EXPECT_TRUE(res.has_value());
}

void expect_serial(NJson& root, std::function<void(Writer::Serializer&)> fn,
                   bool exp_complete = true) {
  auto exp = Writer::nlohmann_serializer(root);
  EXPECT_TRUE(exp.has_value());

  auto serial = std::move(exp.value());
  fn(serial);

  if (exp_complete) {
    expect_ok_op(serial.close());
  } else {
    expect_invalid_op(serial.close());
  }
}

TEST(NLOH_WRITE, invalid_root) {
  {
    NJson js(nullptr);
    expect_invalid_serial(js);
  }
  {
    NJson js(false);
    expect_invalid_serial(js);
  }
  {
    NJson js("bob"sv);
    expect_invalid_serial(js);
  }
  {
    NJson js(23.01);
    expect_invalid_serial(js);
  }
}

TEST(NLOH_WRITE, invalid_arrays_ops) {
  NJson jsarr = NJson::array();
  expect_serial(jsarr, [](auto& serial) {
    expect_invalid_op(serial.end_array());
    expect_invalid_op(serial.end_object());
  });
}

TEST(NLOH_WRITE, invalid_object_ops) {
  NJson jsobj = NJson::object();
  expect_serial(jsobj, [](auto& serial) {
    expect_invalid_op(serial.end_array());
    expect_invalid_op(serial.start_object());
    expect_invalid_op(serial.end_object());
    expect_invalid_op(serial.write_bool(false));
  });
}

TEST(NLOH_WRITE, primitive_arrays) {
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.write_bool(true));
    });

    EXPECT_EQ(jsarr.size(), 1);
    EXPECT_EQ(jsarr[0], true);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.write_i64(1));
      expect_ok_op(serial.write_i64(2));
    });

    EXPECT_EQ(jsarr.size(), 2);
    EXPECT_EQ(jsarr[0], 1);
    EXPECT_EQ(jsarr[1], 2);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.write_double(1.0));
      expect_ok_op(serial.write_str("bob"sv));
      expect_ok_op(serial.write_bool(false));
    });

    EXPECT_EQ(jsarr.size(), 3);
    EXPECT_EQ(jsarr[0], 1.0);
    EXPECT_EQ(jsarr[1], "bob"sv);
    EXPECT_EQ(jsarr[2], false);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.write_null());
    });

    EXPECT_EQ(jsarr.size(), 1);
    EXPECT_EQ(jsarr[0], nullptr);
  }
}

TEST(NLOH_WRITE, primitive_objects) {
  {
    NJson jsobj = NJson::object();
    expect_serial(jsobj, [](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(true));
    });

    EXPECT_EQ(jsobj.size(), 1);
    EXPECT_EQ(jsobj["bob"sv], true);
  }
  {
    NJson jsobj = NJson::object();
    expect_serial(jsobj, [](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_u64(29));

      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_u64(32));
    });

    EXPECT_EQ(jsobj.size(), 2);
    EXPECT_EQ(jsobj["bob"sv], 29);
    EXPECT_EQ(jsobj["alice"sv], 32);
  }
  {
    NJson jsobj = NJson::object();
    expect_serial(jsobj, [](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_u64(29));

      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_null());

      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(false));
    });

    EXPECT_EQ(jsobj.size(), 2);
    EXPECT_EQ(jsobj["bob"sv], false);
    EXPECT_EQ(jsobj["alice"sv], nullptr);
  }
}

TEST(NLOH_WRITE, array_of_items) {
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.end_object());
    });
    EXPECT_EQ(jsarr.size(), 1);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.write_bool(false));
      expect_ok_op(serial.end_object());
    });

    EXPECT_EQ(jsarr.size(), 1);
    auto& jsobj = jsarr[0];
    EXPECT_EQ(jsobj.size(), 1);
    EXPECT_EQ(jsobj["bob"sv], false);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      for (int i = 0; i < 4; ++i) {
        expect_ok_op(serial.start_object());
        expect_ok_op(serial.end_object());
      }
    });
    EXPECT_EQ(jsarr.size(), 4);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.start_array());
      expect_ok_op(serial.write_bool(true));
      expect_ok_op(serial.end_array());
    });

    EXPECT_EQ(jsarr.size(), 1);
    auto& subarr = jsarr[0];
    EXPECT_EQ(subarr.size(), 1);
    EXPECT_EQ(subarr[0], true);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.start_array());
      for (int i = 0; i < 20; ++i) {
        expect_ok_op(serial.write_u64(i));
      }
      expect_ok_op(serial.end_array());
    });

    EXPECT_EQ(jsarr.size(), 1);
    auto& subarr = jsarr[0];
    EXPECT_EQ(subarr.size(), 20);
    for (int i = 0; i < 20; ++i) {
      EXPECT_EQ(subarr[i], i);
    }
  }
}

TEST(NLOH_WRITE, object_of_items) {
  {
    NJson jsobj = NJson::object();
    expect_serial(jsobj, [](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.start_array());
      for (int i = 0; i < 20; ++i) {
        expect_ok_op(serial.write_u64(i));
      }
      expect_ok_op(serial.end_array());
    });

    EXPECT_EQ(jsobj.size(), 1);
    auto& subarr = jsobj["bob"sv];
    EXPECT_EQ(subarr.size(), 20);
    for (int i = 0; i < 20; ++i) {
      EXPECT_EQ(subarr[i], i);
    }
  }
  {
    NJson jsobj = NJson::object();
    expect_serial(jsobj, [](auto& serial) {
      expect_ok_op(serial.write_key("bob"sv));
      expect_ok_op(serial.start_object());
      expect_ok_op(serial.write_key("alice"sv));
      expect_ok_op(serial.write_bool(false));
      expect_ok_op(serial.end_object());
    });

    EXPECT_EQ(jsobj.size(), 1);
    auto& subobj = jsobj["bob"sv];
    EXPECT_EQ(subobj["alice"sv], false);
  }
  {
    // Closing an object with a pending key
    NJson jsobj = NJson::object();
    expect_serial(
        jsobj,
        [](auto& serial) {
          expect_ok_op(serial.write_key("bob"sv));
          expect_ok_op(serial.start_object());
          expect_ok_op(serial.write_key("alice"sv));
          expect_invalid_op(serial.end_object());
        },
        false);
  }
}

TEST(NLOH_WRITE, raw_data) {
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      expect_ok_op(serial.write(Data::JsonValue(true)));
      expect_ok_op(serial.write(Data::JsonValue(false)));
      expect_ok_op(serial.write(Data::JsonValue(uint64_t(25))));
    });

    EXPECT_EQ(jsarr.size(), 3);
    EXPECT_EQ(jsarr[0], true);
    EXPECT_EQ(jsarr[1], false);
    EXPECT_EQ(jsarr[2], 25);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      Data::JsonArray arr;
      expect_ok_op(serial.write(arr));
    });

    EXPECT_EQ(jsarr.size(), 1);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      Data::JsonArray arr;
      for (int i = 0; i < 4; ++i) {
        arr.internal().emplace_back(uint64_t(i));
      }
      expect_ok_op(serial.write(arr));
    });

    EXPECT_EQ(jsarr.size(), 1);
    auto& subarr = jsarr[0];
    for (int i = 0; i < 4; ++i) {
      EXPECT_EQ(subarr[i], i);
    }
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      Data::JsonObject obj;
      expect_ok_op(serial.write(obj));
      expect_ok_op(serial.write(Data::JsonValue(obj)));
    });

    EXPECT_EQ(jsarr.size(), 2);
  }
  {
    NJson jsarr = NJson::array();
    expect_serial(jsarr, [](auto& serial) {
      Data::JsonObject obj;
      obj.internal().insert({std::string("bob"sv), Data::JsonValue(true)});
      obj.internal().insert(
          {std::string("alice"sv), Data::JsonValue("gary"sv)});
      expect_ok_op(serial.write(obj));
    });

    EXPECT_EQ(jsarr.size(), 1);
    auto& subobj = jsarr[0];
    EXPECT_EQ(subobj.size(), 2);
    EXPECT_EQ(subobj["bob"sv], true);
    EXPECT_EQ(subobj["alice"sv], "gary"sv);
  }
}

#endif
