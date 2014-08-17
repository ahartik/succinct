#include <gtest/gtest.h>
#include <iostream>

#include "wt-csa.hpp"
#include "csa.hpp"
#include "rle-delta-vector.hpp"
#include "sparse-vector.hpp"
#include "rle-sparse-vector.hpp"
#include "suffix-array.hpp"

template<typename T>
class CSATest : public ::testing::Test {

};

typedef ::testing::Types<
  CSA<DeltaVector<>>,
  CSA<RLEDeltaVector<>>,
  CSA<SparseVector>,
  CSA<RLESparseVector>,
  WtCSA,
  SuffixArray
  > CSATypes;

TYPED_TEST_CASE(CSATest, CSATypes);

TYPED_TEST(CSATest, Locate) {
  std::string text = "antero antaa ananasta";
  TypeParam sa(text.c_str(), text.size());
  
  std::string pattern = "an";
  auto range = sa.locate(pattern);
  for (auto i = range.first; i < range.second; ++i) {
    int p = sa.sa(i);
    EXPECT_EQ(text.substr(p, pattern.size()), pattern);
  }
  EXPECT_NE(text.substr(sa.sa(range.second), pattern.size()), pattern);
  EXPECT_NE(text.substr(sa.sa(range.first-1), pattern.size()), pattern);

  // no wraparaund :)
  range = sa.locate("taan");
  EXPECT_EQ(range.first, range.second);
}

TYPED_TEST(CSATest, Nulls) {
  std::string text;
  text += "antero";
  text.push_back(0);
  text += "ananas";
  text.push_back(0);
  text += "antaa";
  text.push_back(0);
  text += "banana";

  TypeParam sa(text.c_str(), text.size());
  
  std::string pattern;
  pattern.push_back(0);
  auto range = sa.locate(pattern);
  for (auto i = range.first; i < range.second; ++i) {
    int p = sa.sa(i);
    EXPECT_EQ(text.substr(p, pattern.size()), pattern);
  }
}
