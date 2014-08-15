#include "delta-vector.hpp"
#include "rle-delta-vector.hpp"
#include "sparse-vector.hpp"
#include "rle-sparse-vector.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>
#include <algorithm>

template<typename T>
class DeltaVectorTest : public ::testing::Test {

};

typedef ::testing::Types<
  DeltaVector<>,
  DeltaVector<128>,
  RLEDeltaVector<>,
  SparseVector,
  RLESparseVector
  > DeltaVectorTypes;
TYPED_TEST_CASE(DeltaVectorTest, DeltaVectorTypes);

TEST(DeltaCoding, EncodeRead) {
  std::vector<uint64_t> bits;
  std::vector<int> nums;
  int n = 100;
  for (int i = 0; i < n; ++i) {
    nums.push_back(1 + rand() % (1 + i));
  }
  DeltaEncoder enc(&bits);
  for (int x : nums) {
    enc.add(x);
  }
  DeltaReader reader(&bits[0], 0);
  for (int i = 0; i < n; ++i) {
    int a = reader.read();
#if 0
    int b = nums[i];
    std::cout << a << " == " << b << "\n";
    assert (a == b);
#endif
    EXPECT_EQ(a, nums[i]);
  }
};


TYPED_TEST(DeltaVectorTest, Access) {
  std::vector<int> nums;
  int n = 2000;
  for (int i = 0; i < n; ++i) {
    nums.push_back(1 + rand() % (1 + i));
  }
  // add many strings of ones
  for (int j = 0; j < 10; ++j) {
    for (int i = 0; i < 20; ++i) {
      nums.push_back(j * 100 + i);
    }
  }
  nums.push_back(410000000);
  std::sort(nums.begin(), nums.end());
  nums.erase(std::unique(nums.begin(), nums.end()), nums.end());
  TypeParam dv(nums);
  EXPECT_EQ(nums.size(), dv.size());

  for (size_t i = 0; i < dv.size(); ++i) {
    EXPECT_EQ(dv[i], nums[i]);
  }
};

TYPED_TEST(DeltaVectorTest, Search) {
  std::vector<int> nums;
  int n = 200;
  srand(0);
  for (int i = 0; i < n; ++i) {
    nums.push_back(1 + rand() % (1 + i));
  }
  for (int i = 100; i < 200; ++i) {
    nums.push_back(i);
  }
  std::sort(nums.begin(), nums.end());
  nums.erase(std::unique(nums.begin(), nums.end()), nums.end());
  TypeParam dv(nums);

  uint64_t check = 0;
  int max = nums.back();
  for (int i = 1; i < max; ++i) {
    int n = i; // 1 + rand() % max;
    int p = dv.lower_bound(n);
    check = check * 31 + p;
    EXPECT_LT(p, dv.size());
    EXPECT_GE(dv[p], n);
    if (p != 0) {
      EXPECT_LT(dv[p-1], n);
    }
  }
  std::cout << "check = " << check << "\n";
};
