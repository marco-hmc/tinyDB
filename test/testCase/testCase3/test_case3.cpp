#include <gtest/gtest.h>

#include "spdlog/spdlog.h"

// 示例测试用例
TEST(Case3Test, ExampleTest) {
    spdlog::info("Running Case2Test ExampleTest");
    EXPECT_EQ(1, 1);
}