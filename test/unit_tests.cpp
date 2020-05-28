#include <gtest/gtest.h>

#include <flow.hpp>

TEST(Combine, Basic) {
  ASSERT_EQ(Flow::combine(0), 0);
  ASSERT_EQ(Flow::combine(0), Flow::combine(0));
  ASSERT_EQ(Flow::combine(1, 2), Flow::combine(1, 2));
  ASSERT_NE(Flow::combine(1, 2), Flow::combine(2, 1));
  ASSERT_EQ(Flow::combine(2, 3), Flow::combine(2, 3));
  ASSERT_NE(Flow::combine(2, 3), Flow::combine(3, 2));
  ASSERT_EQ(Flow::combine(4, 5, 6), Flow::combine(4, 5, 6));
  ASSERT_NE(Flow::combine(4, 5, 6), Flow::combine(6, 5, 4));
}

TEST(Digest, Protocol) {
  using namespace Flow;

  auto e1 = Protocol{IP{123456789, 987654321}};
  auto e2 = Protocol{IP{987654321, 123456789}};

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

TEST(Combine, Associative) {
  using namespace Flow;

  auto h1 = combine(1, 2, 3, 4);
  auto h2 = combine(1);
  h2 = combine(h2, 2);
  h2 = combine(h2, 3);
  h2 = combine(h2, 4);

  auto h3 = combine(1);
  h3 = combine(2, h3);
  h3 = combine(3, h3);
  h3 = combine(4, h3);

  ASSERT_EQ(h1, h2);
  ASSERT_NE(h1, h3);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
