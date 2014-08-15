#include "balanced-wavelet.hpp"
#include "skewed-wavelet.hpp"
#include "rle-wavelet.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
using namespace std;

// TODO implement these in all Wavelet trees.
TEST(BalancedWaveletTest, Select) {
  vector<int> v = {4,2,3,1,2,3,4,5};
  // BalancedWavelet<> wt(v.begin(), v.end(), 3);
  BalancedWavelet<> wt(std::move(v));
  EXPECT_EQ(1, wt.select(1, 4));
  EXPECT_EQ(2, wt.select(1, 2));
  EXPECT_EQ(3, wt.select(1, 3));
  EXPECT_EQ(4, wt.select(1, 1));
  EXPECT_EQ(5, wt.select(2, 2));
}

TEST(BalancedWaveletTest, Indexing) {
  vector<int> v = {4,2,3,1,2,3,4,5,0};
  BalancedWavelet<> wt(v.begin(), v.end(), 3);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], wt[i]) << " i = " << i;
  }
}

template<typename T>
class WaveletTest : public ::testing::Test {

};

typedef ::testing::Types<
  BalancedWavelet<>,
  SkewedWavelet<>,
  RLEWavelet<BalancedWavelet<>>,
  RLEWavelet<SkewedWavelet<>>
  > WaveletTypes;

TYPED_TEST_CASE(WaveletTest, WaveletTypes );
TYPED_TEST(WaveletTest, Rank) {
  vector<int> v = 
  {
    4,2,3,1,2,3,4,5,
    6,6,6,7,7,7,7,7
  };
  TypeParam wt(v.begin(), v.end());
  EXPECT_EQ(0, wt.rank(0, 4));
  EXPECT_EQ(0, wt.rank(2, 5));

  EXPECT_EQ(1, wt.rank(1, 4));
  EXPECT_EQ(1, wt.rank(2, 2));
  EXPECT_EQ(2, wt.rank(5, 2));

  EXPECT_EQ(0, wt.rank(8, 6));
  EXPECT_EQ(1, wt.rank(9, 6));
  EXPECT_EQ(2, wt.rank(10, 6));
  EXPECT_EQ(3, wt.rank(11, 6));

  EXPECT_EQ(0, wt.rank(11, 7));
  EXPECT_EQ(1, wt.rank(12, 7));
}

TYPED_TEST(WaveletTest, RankLE) {
  vector<int> v = 
  {
    4,2,3,1,2,3,4,5,
    6,6,6,0,0,0,0,0
  };
  TypeParam wt(v.begin(), v.end());
  EXPECT_EQ(0, wt.rankLE(0, 4));
  EXPECT_EQ(2, wt.rankLE(2, 5));

  EXPECT_EQ(2, wt.rankLE(3, 3));
  EXPECT_EQ(3, wt.rankLE(5, 2));
  EXPECT_EQ(v.size(), wt.rankLE(v.size(), 7));

  EXPECT_EQ(8, wt.rankLE(8, 6));
  EXPECT_EQ(9, wt.rankLE(9, 6));
  EXPECT_EQ(10, wt.rankLE(10, 6));
  EXPECT_EQ(11, wt.rankLE(11, 6));
  EXPECT_EQ(0, wt.rankLE(11, 0));
  EXPECT_EQ(1, wt.rankLE(12, 0));
}

