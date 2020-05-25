#include <gtest/gtest.h>

#include <flow.hpp>

TEST(Combine, Basic) {
  ASSERT_NE(Flow::combine(0), 0);
  ASSERT_EQ(Flow::combine(0), Flow::combine(0));
  ASSERT_EQ(Flow::combine(1, 2), Flow::combine(1, 2));
  ASSERT_NE(Flow::combine(1, 2), Flow::combine(2, 1));
  ASSERT_EQ(Flow::combine(2, 3), Flow::combine(2, 3));
  ASSERT_NE(Flow::combine(2, 3), Flow::combine(3, 2));
  ASSERT_EQ(Flow::combine(4, 5, 6), Flow::combine(4, 5, 6));
  ASSERT_NE(Flow::combine(4, 5, 6), Flow::combine(6, 5, 4));
}

TEST(Digest, Entry) {
  using namespace Flow;

  auto e1 = Entry{IP{123456789, 987654321}};
  auto e2 = Entry{IP{987654321, 123456789}};

  ASSERT_NE(digest(e1), digest(e2));
}

TEST(Digest, NonCommutative) {
  using namespace Flow;

  auto r1 = Record{};
  auto r2 = Record{};

  r1.push_back(IP{123456789, 987654321});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(digest(r1), digest(r2));

  r1.push_back(TCP{8080, 9669});
  r2.push_back(TCP{9669, 8080});

  ASSERT_NE(digest(r1), digest(r2));
}

TEST(Digest, Record) {
  using namespace Flow;

  auto r1 = Record{};
  auto r2 = Record{};

  r1.push_back(IP{123456789, 987654321});
  r1.push_back(TCP{8080, 9669});

  r2.push_back(TCP{9669, 8080});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(digest(r1), digest(r2));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
