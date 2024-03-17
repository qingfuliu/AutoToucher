#include <gtest/gtest.h>
#include <type_traits>
#include "configurator/auto_config.h"
TEST(test_net_sever, test_net_sever12) {
  constexpr bool p = std::is_constructible_v<typename clsn::ConfigItem<clsn::detail::TypeWrapper>::StringType, char *>;
  EXPECT_EQ(p, true);
  constexpr bool p2 = clsn::ConfigItem<clsn::detail::TypeWrapper>::template SupportedTy<const std::string &>::value;
  EXPECT_EQ(p, true);

  constexpr bool p3 = std::is_same_v<std::string, std::decay_t<const std::string &> >;
  EXPECT_EQ(p3, true);
}