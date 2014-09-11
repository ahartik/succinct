#include "csa.hpp"
#include "rle-delta-vector.hpp"
#include "rle-sparse-vector.hpp"
#include "rrr-bit-vector.hpp"
#include "sparse-vector.hpp"
#include "wt-csa.hpp"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <fstream>

using namespace std;
template<typename SA>
void test(const std::string& str) {
  SA csa(str.c_str(), str.size());
  size_t bs = csa.byteSize();
  std::cout << "CSA::byteSize() = " << bs << "\n";
  std::cout << double(bs) / str.size() << " bytes / char \n";

  uint64_t check = 0;
  const int iters = 10000;
  const int pattern_len = 5;
  srand(0);
  {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    for (size_t i = 0; i < iters; ++i) {
      int p = rand() % str.size();
      check = check * 31 + csa.sa(p);
    }
    auto end = clock.now();
    long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << (ns / iters) << " ns per access \n";
    std::cout << "    check = " << check << "\n";
  }
  {
    std::chrono::high_resolution_clock clock;
    std::vector<std::string> patterns;
    for (int i = 0; i < iters; ++i) {
      size_t pos = rand() % (str.size() - pattern_len);
      string p = str.substr(pos, pattern_len);
      patterns.push_back(p);
    }
    check = 0;
    auto start = clock.now();
    for (size_t i = 0; i < iters; ++i) {
      check = check * 31 + csa.locate(patterns[i]).first;
    }
    auto end = clock.now();
    long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << (ns / iters) << " ns per locate\n";
    std::cout << "    check = " << check << "\n";
  }
}

template<typename SA>
void all() {
  string input;
#if 0
   for (int i = 1; i < 100; i *= 10) {
     const size_t n = 1000 * 1000 * i;
 
     while (input.size() < n) {
       input.push_back(rand() % 256);
     }
     std::cout << "== RANDOM " << n << " ==\n";
     test<SA>(input);
   }
#endif

  std::cout << "== COMPRESSIBLE ==\n";
  std::string text;
  
#ifdef LEN_COUNTERS
  memset(len_counter, 0, sizeof(len_counter));
#endif
  std::ifstream fin("test_text");
  while (fin.good())
    text.push_back(fin.get());
  test<SA>(text);
#ifdef LEN_COUNTERS
  std::cout << "len counters:\n";
  for (int i = 0; i < 65; ++i) {
    if (len_counter[i] != 0)
      std::cout << i << ":\t" << len_counter[i] << " - " << len_counter[i] * i << "\n";
  }
#endif
}

int main() {
   std::cout << "----- CSA<DeltaVector<>>\n";
   all<CSA<DeltaVector<>>>();
   std::cout << "----- CSA<SparseVector>\n";
   all<CSA<SparseVector>>();
   std::cout << "----- CSA<RLESparseVector>\n";
   all<CSA<RLESparseVector>>();
   std::cout << "----- CSA<RLEDeltaVector<128>>\n";
   all<CSA<RLEDeltaVector<128>>>();
//   std::cout << "----- CSA<RLEDeltaVector<512>>\n";
//   all<CSA<RLEDeltaVector<512>>>();
  std::cout << "----- WtCSA \n";
  all<WtCSA<>>();
}
