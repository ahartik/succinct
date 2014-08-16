#include "fast-bit-vector.hpp"
#include "rrr-bit-vector.hpp"
#include <iostream>
#include <random>
#include <chrono>

using namespace std;
template<typename BitVector>
class Test {
 public:
  void Rank(int iters) {
    const size_t size = 1<<25;
    std::cout << "Rank:\n";
    using namespace std::chrono;
    std::mt19937_64 mt(0);
    MutableBitVector v;
    for (size_t j = 0; j < size; ++j) {
      v.push_back(mt()%2);
    }
    BitVector vec(v);
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    unsigned long long total = 0;
    for (int j = 0; j < iters; ++j) {
      total = total * 178923 + 987341;
      total += vec.rank(total % size, total%2);
    }
    std::cout << "total = " << total << endl;
    auto end = clock.now();
    long ms = duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << ms << "ms\n";
    std::cout << duration_cast<nanoseconds>(end-start).count()/iters << "ns/rank\n";
  }

  void RankSparse(int iters) {
    const size_t size = 1<<25;
    std::cout << "Rank sparse:\n";
    const unsigned int rarity = 1<<10;
    using namespace std::chrono;
    std::mt19937_64 mt(1);
    MutableBitVector v;
    for (size_t j = 0; j < size; ++j) {
      v.push_back(mt()%rarity == 0);
    }
    BitVector vec(v);
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    unsigned long long total = 0;
    for (int j = 0; j < iters; ++j) {
      total = total * 178923 + 987341;
      total += vec.rank(total % size, total%2);
    }
    auto end = clock.now();
    std::cout << "total = " << total << endl;
    std::cout << double(vec.byteSize() * 8) / vec.size() << " bits/bit\n";
    long ms = duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << ms << "ms\n";
    std::cout << duration_cast<nanoseconds>(end-start).count()/iters << "ns/rank\n";
  }

  void Select(int iters) {
    const size_t size = 1<<25;
    std::cout << "Select:\n";
    using namespace std::chrono;
    std::mt19937_64 mt(2);
    MutableBitVector v;
    for (size_t j = 0; j < size; ++j) {
      v.push_back(mt()%2);
    }
    BitVector vec(v);
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    unsigned long long total = 0;
    for (int j = 0; j < iters; ++j) {
      bool b = total % 2;
      total = total * 178923 + 987341;
      total += vec.select(total % (size / 4), b);
    }
    auto end = clock.now();
    std::cout << "total = " << total << endl;
    long ms = duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << ms << "ms\n";
    std::cout << duration_cast<nanoseconds>(end-start).count()/iters << "ns/sel\n";
  }

  void SelectSparse(int iters) {
    const size_t size = 1<<25;
    const unsigned int rarity = 1<<7;
    std::cout << "Select sparse:\n";
    using namespace std::chrono;
    std::mt19937_64 mt(3);
    MutableBitVector v;
    for (size_t j = 0; j < size; ++j) {
      v.push_back(mt()%rarity == 0);
    }
    BitVector vec(v);
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    unsigned long long total = 0;
    for (int j = 0; j < iters; ++j) {
      bool b = 1;
      total = total * 178923 + 987341;
      total += vec.select(total % (size / (2*rarity)), b);
    }
    std::cout << "total = " << total << endl;
    auto end = clock.now();
    long ms = duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << ms << "ms\n";
    std::cout << duration_cast<nanoseconds>(end-start).count()/iters << "ns/sel\n";
  }

  void Construct() {
    const size_t size = 1<<25;
    std::cout << "Construct " << size << " bits:\n";
    using namespace std::chrono;
    std::mt19937_64 mt(time(0));
    MutableBitVector v(size);
    for (size_t j = 0; j < size; j += 64) {
      v.setWord(j, mt(), 64);
    }
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    BitVector vec(v);
    std::cout << double(vec.byteSize() * 8) / vec.size() << " bits/bit\n";
    // std::cout << double(vec.extra_bits()) / vec.size() << " extra/bit\n";
    auto end = clock.now();
    long ms = duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << ms << "ms\n";
    long ns = duration_cast<nanoseconds>(end-start).count();
    std::cout << (size / (8 * 1024 * 1024.0)) / (ns / 1000000000.0) << " MB/s\n";
  }
};

template<typename Vec>
void test(size_t iters) {
  Test<Vec> t;
  t.Rank(iters);
  t.Select(iters);
  t.RankSparse(iters);
  t.SelectSparse(iters);
  t.Construct();
}

int main() {
  std::cout << "===  FastBitVector:\n";
  test<FastBitVector>(1000000);
  std::cout << "===  RRRBitVector:\n";
  test<RRRBitVector<63>>(1000000);
}
