#include "skewed-wavelet.hpp"
#include "balanced-wavelet.hpp"
#include "rle-wavelet.hpp"

#include <iostream>
#include <random>
#include <chrono>

using namespace std;

template<typename Wt>
void RankLE(int iters, int m, const char* name) {
  const size_t size = 1<<20;
  const size_t max = 1<<20;
  const size_t max_run = 1024;
  std::cout << name << "::rankLE(" << m << "):\n";
  using namespace std::chrono;
  std::mt19937_64 mt(0);
  std::vector<uint64_t> v;
  while (v.size() < size) {
    uint64_t val = mt() % max;
    int run = 1 + mt() % max_run;
    for (int i = 0; i < run && v.size() < size; ++i) {
      v.push_back(val);
    }
  }
  Wt wt(v.begin(), v.end());
  std::chrono::high_resolution_clock clock;
  auto start = clock.now();
  unsigned long long total = 0;
  for (int j = 0; j < iters; ++j) {
    total = total * 178923 + 987341;
    total += wt.rankLE(total % size, m);
  }
  // Make sure compiler is not too smart.
  std::cout << "(" << total << ")\n";
  
  auto end = clock.now();
  std::cout << duration_cast<nanoseconds>(end-start).count()/iters << "ns/rank\n";
}

int main() {
  int iters = 100000;
  RankLE<BalancedWavelet<>>(iters, 32, "BalancedWavelet");
  RankLE<BalancedWavelet<>>(iters, 1<<10, "BalancedWavelet");
  cout << endl;
  RankLE<SkewedWavelet<>>(iters, 32, "SkewedWavelet");
  RankLE<SkewedWavelet<>>(iters, 1<<10, "SkewedWavelet");
  cout << endl;
  RankLE<RLEWavelet<BalancedWavelet<>>>(iters, 32, "RLEWavelet<BalancedWavelet>");
  RankLE<RLEWavelet<BalancedWavelet<>>>(iters, 1<<10, "RLEWavelet<BalancedWavelet>");
  cout << endl;
  RankLE<RLEWavelet<SkewedWavelet<>>>(iters, 32, "RLEWavelet<SkewedWavelet>");
  RankLE<RLEWavelet<SkewedWavelet<>>>(iters, 1<<10, "RLEWavelet<SkewedWavelet>");
}
