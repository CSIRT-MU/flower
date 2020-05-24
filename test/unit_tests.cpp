#include <gtest/gtest.h>

#include "../include/flow.hpp"

TEST(Combine, Basic) {
  ASSERT_NE(Flow::combine(0), 0);
  ASSERT_EQ(Flow::combine(0), Flow::combine(0));
  ASSERT_EQ(Flow::combine(1, 2), Flow::combine(1, 2));
  ASSERT_EQ(Flow::combine(2, 3), Flow::combine(2, 3));
  ASSERT_EQ(Flow::combine(4, 5, 6), Flow::combine(4, 5, 6));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
