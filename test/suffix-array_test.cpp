#include <gtest/gtest.h>
#include <iostream>

#include "suffix-array.hpp"

TEST(SuffixArray, Locate) {
  std::string text = "antero antaa ananasta";
  SuffixArray sa(text.c_str(), text.size());
  
  std::string pattern = "an";
  auto range = sa.locate(pattern);
  for (auto i = range.first; i < range.second; ++i) {
    int p = sa.sa(i);
    EXPECT_EQ(text.substr(p, pattern.size()), pattern);
  }
  EXPECT_NE(text.substr(sa.sa(range.second), pattern.size()), pattern);
  EXPECT_NE(text.substr(sa.sa(range.first-1), pattern.size()), pattern);
}

TEST(SuffixArray, LCP) {
  std::string text = "antero antaa ananasta";
  SuffixArray sa(text.c_str(), text.size());
  for (size_t i = 1; i < text.size(); ++i) {
    int lcp = sa.lcp(i);
    int a = sa.sa(i);
    int b = sa.sa(i-1);
    EXPECT_EQ(text.substr(a, lcp),
              text.substr(b, lcp));
    EXPECT_NE(text.substr(a, lcp+1),
              text.substr(b, lcp+1));
  }
}
