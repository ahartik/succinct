#pragma once

#include "fast-bit-vector.hpp"
#include "int-array.hpp"

#include <iterator>
#include <stdint.h>
#include <cmath>
#include <cstring>

class SparseBitVector {
 public:
  // Empty constructor
  SparseBitVector()
    : pop_(0),
      size_(0)
  {}

  template<typename It>
  SparseBitVector(It begin, It end) {
    init(begin, end);
  }
  SparseBitVector(const MutableBitVector& vec) {
    std::vector<size_t> pos;
    for (size_t i = 0; i < vec.size(); ++i) {
      if (vec[i]) pos.push_back(i);
    }
    init(pos.begin(), pos.end());
  }

  SparseBitVector(SparseBitVector&& o) : SparseBitVector() {
    swap(*this, o);
  }

  const SparseBitVector& operator=(SparseBitVector&& o) {
    swap (*this, o);
    return *this;
  }

  bool operator[](size_t pos) const {
    uint64_t mask = (1LL << w_) - 1;
    size_t high = pos >> w_;
    size_t low = pos & mask;
    size_t y = high_bits_.select(high, 0);
    size_t x = y - high;
    for (;high_bits_[y] == 1; x++, y++) {
      // TODO
      // perhaps binary search here
      size_t l = this->low(x);
      if (l >= low) {
        if (l == low) return 1;
        return 0;
      }
    }
    return 0;
  }

  size_t rank(size_t pos, bool bit) const {
    if (pos == 0) return 0;
    if (size_ == 0) return 0;
    if (pos >= size_) return count(bit);
    uint64_t mask = (1LL << w_) - 1;
    size_t high = pos >> w_;
    size_t low = pos & mask;
    size_t y = high_bits_.select(high, 0);
    size_t x = y - high;
    for (;high_bits_[y] == 1; x++, y++) {
      // TODO
      // perhaps binary search here
      size_t l = this->low(x);
      if (l >= low) {
        // if (l == low) ++x;
        break;
      }
    }
    return bit ? x : pos - x;
  }

  // Compatibility function, for b = 0 binary search is used.
  size_t select(size_t rnk, bool b) const {
    if (rnk == 0) return 0;
    if (b == 0)
    {
      size_t left = 0;
      size_t right = size_;
      while (left != right - 1) {
        size_t c = (left + right) / 2;
        if (rank(c, 0) < rnk) {
          left = c;
        } else {
          right = c;
        }
      }
      return left + 1;
    }
    return select1(rnk);
  }
  size_t select1(size_t rank) const {
    if (rank == 0) return 0;
    return ((high_bits_.select(rank, 1) - rank) << w_) + low(rank-1) + 1;
  }

  size_t count(bool bit) const {
    if (bit) return pop_;
    return size_ - pop_;
  }

  size_t bitSize() const {
    return w_ * pop_ + high_bits_.bitSize();
  }

  size_t byteSize() const {
    return  sizeof(*this) + low_arr_.byteSize() + high_bits_.byteSize();
  }

  size_t size() const {
    return size_;
  }

 private:
  size_t calc_size(int w, size_t n, size_t m) {
    return w * m + m + (n >> w);
  }
  template<typename It>
  void init(It begin, It end) {
    size_t m = std::distance(begin, end);
    if (m == 0) {
      // special case for empty bitvectors
      pop_ = 0;
      w_ = 0;
      size_ = 0;
      return;
    }

    pop_ = m;
    It last = end;
    size_t n = 1 + *(--last);
    size_ = n;

    w_ = 2;
    while (w_ < 32 && calc_size(w_, n, m) > calc_size(w_ + 1, n, m)) {
      w_ ++;
    }
    low_arr_ = IntArray(w_, m);
    size_t i = 0;
    uint64_t mask = (1LL << w_) - 1;
    MutableBitVector high_bits(m + (n >> w_));
    for (It it = begin; it != end; ++it) {
      uint64_t pos = *it;
      assert(pos < n);
      low_arr_.set(i, pos & mask);

      uint64_t high = pos >> w_;
      high_bits[high + i] = 1;
      ++i;
    }
    high_bits_ = FastBitVector(high_bits);
  }
  int low(size_t i) const {
    return low_arr_.get(i);
  }
  int w_;
  size_t pop_;
  size_t size_;
  IntArray low_arr_;
  FastBitVector high_bits_;

 public:
  friend void swap(SparseBitVector& a, SparseBitVector& b) {
    using std::swap;
    swap(a.w_, b.w_);
    swap(a.pop_, b.pop_);
    swap(a.size_, b.size_);
    swap(a.low_arr_, b.low_arr_);
    swap(a.high_bits_, b.high_bits_);
  }
};
