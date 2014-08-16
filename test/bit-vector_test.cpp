#include<gtest/gtest.h>

#include <random>
#include "mutable-bit-vector.hpp"
#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "rrr-bit-vector.hpp"


template<typename T>
class BitVectorTest : public ::testing::Test {

};

typedef ::testing::Types<
  FastBitVector,
  SparseBitVector,
  RRRBitVector<63>
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
  MutableBitVector v(128);
  for (int i = 0; i < v.size(); ++i) {
    v[i] = i % 17 == 0;
  }
  TypeParam vec(v);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(int(v[i]), int(vec[i]));
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
}

class RRRBitVectorTest : public ::testing::Test {
 public:
  template<int B>
  Word encode(int k, Word w) const {
    return RRRBitVector<B>::encode(k, w);
  }
  template<int B>
  Word decode(int k, Word w) const {
    return RRRBitVector<B>::decode(k, w);
  }
};

TEST_F(RRRBitVectorTest, EncodeDecode) {
  size_t T = 1000;
  srand(0);
  for (int i = 0; i < T; ++i) {
    Word r = rand() | (Word(rand()) << 32);
    Word w = r % (1ull << 63);
    int k = WordPopCount(w);
    Word code = encode<63>(k, w);
    Word dec = decode<63>(k, code);
    ASSERT_EQ(w, dec);
  }
}
TEST_F(RRRBitVectorTest, Index) {
  MutableBitVector v(128 * 20);
  for (int i = 0; i < v.size(); ++i) {
    v[i] = i % 17 == 0;
  }
  for (int i = 10; i < 10 + 128; ++i)
    v[i] = 1;
  for (int i = 100; i < 100 + 128; ++i)
    v[i] = 0;
  RRRBitVector<63> vec(v);
  for (size_t i = 0; i < v.size(); ++i) {
    ASSERT_EQ(int(v[i]), int(vec[i])) << i;
  }
}
