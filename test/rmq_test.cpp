#include "rmq.hpp"
#include <gtest/gtest.h>
#include <cassert>
#include <algorithm>
#include <random>
#include <vector>

template<typename T>
class RMQTest : public ::testing::Test {
 public:
};

typedef ::testing::Types<
  FastRMQ<int>,
  RMQ<int>
  > RMQTypes;

TYPED_TEST_CASE(RMQTest, RMQTypes);

TYPED_TEST(RMQTest, MinPos) {
  const int kArrsize = 10000;
  const int kIters = 1000;
  std::mt19937 mt(0);
  std::vector<int> vec(kArrsize);
  for (int i = 0; i < kArrsize; ++i)
    vec[i] = mt();
  int* arr = &vec[0];
  TypeParam rmq(arr, kArrsize);
  for (int i = 0; i < kIters; ++i) {
    int l = mt() % kArrsize;
    int r = l + 1 + mt() % (kArrsize - l);
    int mp = rmq.rmq(l,r);
    int real_min = std::min_element(arr + l, arr + r) - arr;
    EXPECT_EQ(mp, real_min);
    if (mp != real_min) {
      std::cout << "From " << l << " to " << r << "\n";
      std::cout << "Error: " << "rmq = " << arr[mp] << " real = " << arr[real_min] << "\n";
    }
  }
}
