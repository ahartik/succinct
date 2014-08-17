#include "int-array.hpp"

#include <gtest/gtest.h>
#include <vector>
#include <cassert>
#include <iostream>
#include <random>

using namespace std;

TEST(IntArrayTest, Index) {
  for (int w = 1; w <= 64; ++w) {
    IntArray arr(w, 100000);
    vector<uint64_t> ref(arr.size());
    for (size_t i = 0; i < arr.size(); ++i) {
      int j = rand() % arr.size();
      uint64_t v = rand() % arr.maxValue();
      arr.set(j, v);
      ref[j] = v;
      ASSERT_EQ(ref[j], arr.get(j)) << "w = " << w;
    }
    for (size_t i = 0; i < arr.size(); ++i) {
      ASSERT_EQ(ref[i], arr.get(i)) << "w = " << w;
    }
  }
}
