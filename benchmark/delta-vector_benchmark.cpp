#include "delta-vector.hpp"
#include "rle-delta-vector.hpp"
#include "sparse-vector.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <vector>

std::vector<int> input;
const int n = 1 << 25;
template<typename DV>
void test() {
  DV dv(input);
  size_t bs = dv.byteSize();
  std::cout << "byteSize() = " << bs << "\n";
  std::cout << double(bs) / input.size() << " bytes / int \n";
  uint64_t check = 0;
  const int iters = 100000;
  srand(0);
  {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    for (size_t i = 0; i < iters; ++i) {
      int p = rand() % input.size();
      check += dv[p];
    }
    auto end = clock.now();
    long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << (ns / iters) << " ns per lookup \n";
    std::cout << "    check = " << check << "\n";
  }
  {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    for (size_t i = 0; i < iters; ++i) {
      int p = rand() % input.size();
      int x = input[p];
      check += dv.lower_bound(x);
    }
    auto end = clock.now();
    long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << (ns / iters) << " ns per lower_bound\n";
    std::cout << "    check = " << check << "\n";
  }
}
int main() {
  for (int i = 0; i < n; ++i) {
    input.push_back(rand());
  }
  std::sort(input.begin(), input.end());
  input.erase(std::unique(input.begin(), input.end()), input.end());


#define TEST(t) \
    do {\
      std::cout << #t << "\n"; \
      test<t>();\
    } while (0)

  TEST(DeltaVector<64>);
  TEST(RLEDeltaVector<>);
  TEST(SparseVector);
}
