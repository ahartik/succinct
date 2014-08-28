#pragma once

#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include <vector>
#include <cassert>

class RLEBitVector {
 public:
  RLEBitVector() {
    size_ = 0;
  }
  RLEBitVector(const MutableBitVector& vec) {
    size_ = vec.size();
    std::vector<size_t> pos;
    std::vector<size_t> rank;
    size_t r = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
      if (vec[i]) {
        pos.push_back(i);
        rank.push_back(r);
        while (i < vec.size() && vec[i]) {
          ++i;
          ++r;
        }
        --i;
      }
    }
    popcount_ = r;
    pos_ = SparseBitVector(pos.begin(), pos.end());
    rank_ = SparseBitVector(rank.begin(), rank.end());
  }
  RLEBitVector(RLEBitVector&& vec) = default;
  RLEBitVector(const RLEBitVector& vec) = default;
  RLEBitVector& operator=(const RLEBitVector& vec) = default;
  RLEBitVector& operator=(RLEBitVector&& vec) = default;

  bool operator[](size_t x) const {
    assert(x < size_);
    ++x;
    size_t runs = pos_.rank(x, 1);
    size_t c0 = rs(runs);
    size_t c1 = rs(runs + 1);
    size_t rpos = pos_.select1(runs) - 1;
    return c0 + x - rpos <= c1;
  }

  size_t rank(size_t i, bool b) const {
    if (b) return rank1(i);
    else return i - rank1(i);
  }
  size_t rank1(size_t x) const {
    size_t runs = pos_.rank(x, 1);
    size_t c0 = rs(runs);
    size_t c1 = rs(runs + 1);
    size_t rpos = pos_.select1(runs) - 1;
    return std::min(c0 + x - rpos, c1);
  }

  size_t select(size_t rnk, bool b) const {
    if (rnk == 0) return 0;
    if (b == 0) {
      size_t left = 0;
      size_t right = size_;
      while (left != right - 1) {
        size_t c = (left + right) / 2;
        if (rank(c, b) < rnk) {
          left = c;
        } else {
          right = c;
        }
      }
      return left + 1;
    }
    return select1(rnk);
  }
  size_t select1(size_t i) const {
    if (i == 0) return 0;
    size_t run = rank_.rank(i, 1);
    size_t pos = pos_.select1(run) - 1;
    size_t rank = rank_.select1(run) - 1;
    return pos + i - rank;
  }
  size_t byteSize() const {
    return sizeof(*this) + 
        (pos_.byteSize() + rank_.byteSize());
  }
  size_t size() const {
    return size_;
  }

 private:
  size_t rs(size_t x) const {
    if (x == 0) return 0;
    if (rank_.count(1) < x) return popcount_;
    return rank_.select1(x) - 1;
  }
  SparseBitVector pos_;
  SparseBitVector rank_;
  size_t popcount_;
  size_t size_;
};
