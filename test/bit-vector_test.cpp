#include<gtest/gtest.h>

#include <random>
#include "mutable-bit-vector.hpp"
#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "rrr-bit-vector.hpp"
#include "rle-bit-vector.hpp"
#include "delta-bit-vector.hpp"
#include "rle-delta-vector.hpp"

template<typename T>
class BitVectorTest : public ::testing::Test {

};

typedef ::testing::Types<
  FastBitVector,
  SparseBitVector,
  RRRBitVector<63>,
  RRRBitVector<32>,
  RLEBitVector,
  DeltaBitVector<>,
  DeltaBitVector<RLEDeltaVector<>>
  > BitVectorTypes;

TYPED_TEST_CASE(BitVectorTest, BitVectorTypes);

TYPED_TEST(BitVectorTest, ShortRank) {
  MutableBitVector v = {false, true, true, false, true};
  TypeParam vec(v);
  EXPECT_EQ(0, vec.rank(0, 1));
  EXPECT_EQ(0, vec.rank(0, 0));
  EXPECT_EQ(1, vec.rank(2, 1));
  EXPECT_EQ(1, vec.rank(2, 0));
  EXPECT_EQ(2, vec.rank(3, 1));
  EXPECT_EQ(1, vec.rank(3, 0));
}

TYPED_TEST(BitVectorTest, RandomRank) {
  std::mt19937_64 mt(0);
  int n = 1<<16;
  int m = n;
  MutableBitVector v(n);
  for (int j = 0; j < n; ++j) {
    v[j] = mt() % 128 == 0;
  }

  TypeParam vec(v);
  int rank = 0;
  for (int j = 0; j < m; ++j) {
    ASSERT_EQ(rank, vec.rank(j, 1)) << " j = " << j;
    rank += v[j];
  }
  ASSERT_EQ(rank, vec.rank(n, 1));
}

TYPED_TEST(BitVectorTest, ShortSelect) {
  MutableBitVector v = {false, true, true, false, true};
  TypeParam vec(v);
  EXPECT_EQ(0, vec.select(0, 1));
  EXPECT_EQ(0, vec.select(0, 0));

  EXPECT_EQ(2, vec.select(1, 1));
  EXPECT_EQ(1, vec.select(1, 0));

  EXPECT_EQ(3, vec.select(2, 1));
  EXPECT_EQ(4, vec.select(2, 0));
}

TYPED_TEST(BitVectorTest, RandomSelect) {
  std::mt19937_64 mt(0);
  int n = 1<<15;
  int m = n / 4;
  MutableBitVector v;
  for (int j = 0; j < n; ++j) {
    v.push_back(mt()%2);
  }
  TypeParam vec(v);
  int rank[2] = {0,0};
  for (int j = 0; j < m; ++j) {
    int b = v[j];
    rank[b]++;
    ASSERT_EQ(j+1, vec.select(rank[b],b)) << j;
  }
}

TYPED_TEST(BitVectorTest, Index) {
  MutableBitVector v(1024);
  for (int i = 0; i < v.size(); ++i) {
    if (i < v.size() / 2) {
      v[i] = i % 17 == 0; 
    } else {
      v[i] = i % 17 != 0; 
    }
  }
  TypeParam vec(v);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(int(v[i]), int(vec[i])) << i;
  }
}

TYPED_TEST(BitVectorTest, Edges) {
  int n = 1<<16;
  MutableBitVector v(n, true);
  TypeParam vec(v);
  // This works even without zeros.
  ASSERT_EQ(0, vec.select(0, 0)); 
  // especially 0 == vec.rank(0,1)
  // and vec.rank(n,1) = n
  for (int j = 0; j <= n; ++j) {
    if (j < n) ASSERT_EQ(1, vec[j]) << j;
    ASSERT_EQ(j, vec.rank(j, 1)) << j;
    ASSERT_EQ(0, vec.rank(j, 0)) << j;
    ASSERT_EQ(j, vec.select(j, 1)) << j;
  }
  // Construct empty
  v.resize(0);
  TypeParam evec(v);
  ASSERT_EQ(0, vec.select(0,0));
  ASSERT_EQ(0, vec.select(0,1));
  ASSERT_EQ(0, vec.rank(0,1));
  ASSERT_EQ(0, vec.rank(0,0));
}
