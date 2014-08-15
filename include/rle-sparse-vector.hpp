#pragma once
#include "sparse-bit-vector.hpp"

// Class compatible with deltavector using SparseBitVectors
class RLESparseVector {
 public:
  template<typename IntT>
  RLESparseVector(const std::vector<IntT>& vec) {
    std::vector<uint64_t> val;
    std::vector<uint64_t> pos;
    for (size_t i = 0; i < vec.size(); ) {
      uint64_t last = vec[i];
      val.push_back(last);
      pos.push_back(i);
      for (++i; vec[i] - last == 1 && i < vec.size(); ++i) {
        last = vec[i];
      }
    }
    pos.push_back(vec.size());
    if (vec.empty())
      val.push_back(1);
    else
      val.push_back(vec.back() + 1);
    size_ = vec.size();
    val_ = SparseBitVector(val.begin(), val.end());
    pos_ = SparseBitVector(pos.begin(), pos.end());
    size_ = vec.size();
  }

  RLESparseVector(const RLESparseVector& v) = default;
  RLESparseVector(RLESparseVector&& v)
      : val_(std::move(v.val_)),
        pos_(std::move(v.pos_)),
        size_(v.size_)
  {
    v.size_ = 0;
  }

  size_t size() const {
    return size_;
  }

  uint64_t operator[](size_t i) const {
    size_t x = pos_.rank(i + 1, 1) - 1;
    uint64_t v = val_.select1(x + 1) - 1;
    uint64_t v_pos = pos_.select1(x + 1) - 1;
    return v + i - v_pos;
  }

  // Finds the first position i where (*this)[i] >= x
  size_t lower_bound(uint64_t x) const {
    if (x >= val_.size()) return size();
    size_t b = val_.rank(x + 1, 1);
    if (b == 0) return 0;
    b--;
    size_t pos = pos_.select1(b + 1) - 1;
    if (pos >= size()) return size();
    uint64_t val = val_.select1(b + 1) - 1;
    assert (val <= x);
    size_t p1 = pos + x - val;
    size_t p2 = pos_.select1(b + 2) - 1;
    return std::min(p1, p2);
  }

  size_t byteSize() const {
    return sizeof(*this) + 
        (val_.bitSize() + pos_.bitSize()) / 8;
  }
  size_t sampleSize() const {
    return 0;
  }
 private:
  SparseBitVector val_;
  SparseBitVector pos_;
  size_t size_;
};

