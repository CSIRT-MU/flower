#include <gtest/gtest.h>

#include <definition.hpp>
#include <serializer.hpp>

const static auto def_all = Flow::Definition{
  {true, true, true},
  {true, true, true},
  {true, true, true},
  {true, true}
};

TEST(Combine, Basic) {
  ASSERT_EQ(combine(0), 0);
  ASSERT_EQ(combine(0), combine(0));
  ASSERT_EQ(combine(1, 2), combine(1, 2));
  ASSERT_NE(combine(1, 2), combine(2, 1));
  ASSERT_EQ(combine(2, 3), combine(2, 3));
  ASSERT_NE(combine(2, 3), combine(3, 2));
  ASSERT_EQ(combine(4, 5, 6), combine(4, 5, 6));
  ASSERT_NE(combine(4, 5, 6), combine(6, 5, 4));
}

TEST(Digest, Protocol) {
  using namespace Flow;
  auto ser = Serializer();
  ser.set_definition(def_all);

  auto e1 = Chain{IP{123456789, 987654321}};
  auto e2 = Chain{IP{987654321, 123456789}};

  ASSERT_NE(ser.digest(e1), ser.digest(e2));
}

TEST(Digest, NonCommutative) {
  using namespace Flow;
  auto ser = Serializer();
  ser.set_definition(def_all);

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(IP{123456789, 987654321});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(ser.digest(r1), ser.digest(r2));

  r1.push_back(TCP{8080, 9669});
  r2.push_back(TCP{9669, 8080});

  ASSERT_NE(ser.digest(r1), ser.digest(r2));
}

TEST(Digest, Record) {
  using namespace Flow;
  auto ser = Serializer();
  ser.set_definition(def_all);

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(IP{123456789, 987654321});
  r1.push_back(TCP{8080, 9669});

  r2.push_back(TCP{9669, 8080});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(ser.digest(r1), ser.digest(r2));
}

TEST(Digest, UsingType) {
  using namespace Flow;
  auto ser = Serializer();
  ser.set_definition(def_all);

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(TCP{1000, 2000});
  r1.push_back(UDP{8080, 9669});

  r2.push_back(UDP{1000, 2000});
  r2.push_back(TCP{8080, 9669});

  ASSERT_NE(ser.digest(r1), ser.digest(r2));
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

TEST(Type, RecordDifferent) {
  using namespace Flow;

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(IP{123456789, 987654321});
  r1.push_back(TCP{8080, 9669});

  r2.push_back(TCP{9669, 8080});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(type(r1), type(r2));
}

TEST(Type, RecordSame) {
  using namespace Flow;

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(IP{123456789, 987654321});
  r1.push_back(TCP{8080, 9669});

  r2.push_back(IP{987654321, 123456789});
  r2.push_back(TCP{9669, 8080});

  ASSERT_EQ(type(r1), type(r2));
}

TEST(Type, RecordDifferentLength) {
  using namespace Flow;

  auto r1 = Chain{};
  auto r2 = Chain{};

  r1.push_back(IP{123456789, 987654321});
  r1.push_back(TCP{8080, 9669});

  r2.push_back(IP{987654321, 123456789});
  r2.push_back(TCP{9669, 8080});
  r2.push_back(TCP{9669, 8080});
  r2.push_back(IP{987654321, 123456789});

  ASSERT_NE(type(r1), type(r2));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
