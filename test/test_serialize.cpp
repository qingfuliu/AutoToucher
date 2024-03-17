//
// Created by lqf on 24-3-15.
//
#include <gtest/gtest.h>
#include <iostream>
#include <type_traits>
#include "configurator/auto_config.h"

class UserCustomT {
 public:
  UserCustomT() = default;

  void DeSerialize(const json::Json::value &json) {
    a = json["a"];
    b = json["b"];
    c = json["c"];
  }
  SERIALIZE("a", a, "b", b, "c", c, "d", d);

  int a{1};
  bool b{false};
  float c{1.222234};
  std::nullptr_t d{nullptr};
};

class UserCustom {
 public:
  UserCustom() = default;

  void DeSerialize(const json::Json::value &json) {
    a = json["a"];
    b = json["b"];
    c = json["c"];
    d = json["d"];
    t = json["t"];
  }
  json::Json::value Serialize() const {
    json::Json::value json;
    json["a"] = a;
    json["b"] = b;
    json["c"] = c;
    json["d"] = d;
    json["t"] = t;
    return json;
  }

  int a{1};
  bool b{false};
  float c{1.222234};
  std::nullptr_t d{nullptr};
  UserCustomT t;
};

class UserCustomA {
 public:
  UserCustomA() = default;
};

class UserCustomB {
 public:
  UserCustomB() = default;
  DESERIALIZE();
};

class UserCustomC {
 public:
  UserCustomC() = default;
  SERIALIZE();
};

TEST(test_serialize, test_serialize1) {
  EXPECT_EQ(1, (clsn::helper::HasMemberDeSerializeFuncV<json::Json::value, UserCustom>));
  EXPECT_EQ(1, (clsn::helper::HasMemberSerializeFuncV<json::Json::value, UserCustom>));
  EXPECT_EQ(0, (clsn::helper::HasMemberDeSerializeFuncV<json::Json::value, UserCustomA>));
  EXPECT_EQ(0, (clsn::helper::HasMemberSerializeFuncV<json::Json::value, UserCustomA>));

  EXPECT_EQ(1, (clsn::helper::HasMemberDeSerializeFuncV<json::Json::value, UserCustomB>));
  EXPECT_EQ(0, (clsn::helper::HasMemberSerializeFuncV<json::Json::value, UserCustomB>));

  EXPECT_EQ(0, (clsn::helper::HasMemberDeSerializeFuncV<json::Json::value, UserCustomC>));
  EXPECT_EQ(1, (clsn::helper::HasMemberSerializeFuncV<json::Json::value, UserCustomC>));

  std::cout << std::is_same_v<int64_t, json::Json::value ::SupportedTyWeak<int>::type> << std::endl;

  auto u = UserCustom{};
  auto json1 = u.Serialize();
  json::Json::Serialize(std::cout, json1);

  auto u2 = UserCustom{};
  u2.a = 234;
  u2.b = true;
  u2.c = 456;
  u2.t.c = 5675;
  u2.t.a = 346;
  u2.t.b = true;
  u2.DeSerialize(json1);
  EXPECT_EQ(u2.a, 1);
  EXPECT_EQ(u2.b, false);
//  EXPECT_EQ(u2.c, 1.222234);
  EXPECT_EQ(u2.t.a, 1);
//  EXPECT_EQ(u2.t.c, 1.222234);
  EXPECT_EQ(u2.t.b, false);
}