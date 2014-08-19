#include <gtest/gtest.h>
#include <iostream>

#include "brute-count.hpp"
#include "ilcp-count.hpp"
#include "rle-ilcp-count.hpp"
#include "rle-wavelet.hpp"
#include "rrr-bit-vector.hpp"
#include "sada-count.hpp"
#include "sada-sparse-count.hpp"

template<typename T>
class CountTest : public ::testing::Test {

};

typedef RRRBitVector<63> RRR;
typedef ::testing::Types<
  ILCPCount<BalancedWavelet<>>,
  ILCPCount<SkewedWavelet<>>,
  ILCPCount<RLEWavelet<BalancedWavelet<>>>,
  ILCPCount<RLEWavelet<SkewedWavelet<>>>,
  ILCPCount<BalancedWavelet<RRR>>,
  ILCPCount<SkewedWavelet<RRR>>,
  ILCPCount<RLEWavelet<BalancedWavelet<RRR>>>,
  ILCPCount<RLEWavelet<SkewedWavelet<RRR>>>,
  SadaCount<FastBitVector>,
  SadaCount<RRR>,
  SadaSparseCount<true>,
  SadaSparseCount<false>,
  RLEILCPCount
  > CountTypes;

TYPED_TEST_CASE(CountTest, CountTypes );

TYPED_TEST(CountTest, Simple) {
  std::string text = "antero\001ananas\001banana";
  SuffixArray sa(text.c_str(), text.size());
  TypeParam counter(sa);
  EXPECT_EQ(counter.count(sa, "an"), 3);
  EXPECT_EQ(counter.count(sa, "b"), 1);
  EXPECT_EQ(counter.count(sa, "tero"), 1);
  EXPECT_EQ(counter.count(sa, "ana"), 2);
}

TEST(CountTest, Brute) {
  std::string text = "antero\001ananas\001banana";
  SuffixArray sa(text.c_str(), text.size());
  BruteCount c(sa);
  EXPECT_EQ(c.count(sa, "an"), 3);
  EXPECT_EQ(c.count(sa, "b"), 1);
  EXPECT_EQ(c.count(sa, "tero"), 1);
  EXPECT_EQ(c.count(sa, "ana"), 2);
}

TEST(CountTest, SadaCount) {
  std::string text = "antero\001ananas\001banana";
  SuffixArray sa(text.c_str(), text.size());
  SadaCount<> c(sa);
  EXPECT_EQ(c.count(sa, "an"), 3);
  EXPECT_EQ(c.count(sa, "b"), 1);
  EXPECT_EQ(c.count(sa, "tero"), 1);
  EXPECT_EQ(c.count(sa, "ana"), 2);
}

TYPED_TEST(CountTest, CompareToBrute) {
  std::string text = 
      "ananas\001"
      "amsterdam\001"
      "apple\001"
      "banana\001"
      "cantelope\001"
      "cat\001"
      "dog\001"
      "dublin\001";
  SuffixArray sa(text.c_str(), text.size());
  TypeParam counter(sa);
  BruteCount brute(sa);
  // test for each short substring
  for (int len = 1; len <= 4; ++ len) {
    for (size_t i = 0; i < text.size(); ++i) {
      std::string pattern = text.substr(i, len);
      if (pattern.find("\001") != std::string::npos) continue;
      EXPECT_EQ(brute.count(sa, pattern),
                counter.count(sa, pattern))
          << "pattern = " << pattern << "\n";
    }
  }
}
