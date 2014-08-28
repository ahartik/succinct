#pragma once

#include "delta-vector.hpp"
#include <vector>
#include <cassert>

template <typename DVec = DeltaVector<>>
class DeltaBitVector {
 public:
  DeltaBitVector() {
    size_ = 0;
  }
  DeltaBitVector(const MutableBitVector& vec) {
    size_ = vec.size();
    std::vector<size_t> pos;
    for (size_t i = 0; i < vec.size(); ++i) {
      if (vec[i]) {
        pos.push_back(i);
      }
    }
    vec_ = DVec(pos);
  }
  DeltaBitVector(DeltaBitVector&& vec) = default;
  DeltaBitVector(const DeltaBitVector& vec) = default;
  DeltaBitVector& operator=(const DeltaBitVector& vec) = default;
  DeltaBitVector& operator=(DeltaBitVector&& vec) = default;

  bool operator[](size_t x) const {
    size_t i = vec_.lower_bound(x);
    return vec_[i] == x;
  }

  size_t rank(size_t i, bool b) const {
    if (b) return rank1(i);
    else return i - rank1(i);
  }
  size_t rank1(size_t x) const {
    return vec_.lower_bound(x);
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
    return vec_[i - 1] + 1;
  }
  size_t byteSize() const {
    return 8 + vec_.byteSize();
  }
  size_t size() const {
    return size_;
  }

 private:
  DVec vec_;
  size_t size_;
};
