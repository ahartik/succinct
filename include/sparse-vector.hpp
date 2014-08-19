#pragma once
#include "sparse-bit-vector.hpp"

#include <vector>

// Class compatible with deltavector using SparseBitVectors
class SparseVector {
 public:
  template<typename IntT>
  explicit SparseVector(const std::vector<IntT>& vec) : bits_(vec.begin(), vec.end()) {
    size_ = vec.size();
  }

  SparseVector() {
    size_ = 0;
  }

  SparseVector(const SparseVector& v) = default;
  SparseVector(SparseVector&& v)
      : bits_(std::move(v.bits_))
  {
    v.size_ = 0;
  }
  SparseVector& operator=(const SparseVector& v) = default;
  SparseVector& operator=(SparseVector&& v) {
    bits_ = std::move(v.bits_);
    size_ = v.size_;
    v.size_ = 0;
    return *this;
  }

  size_t size() const {
    return size_;
  }

  uint64_t operator[](size_t i) const {
    return bits_.select1(i + 1) - 1;
  }

  // Finds the first position i where (*this)[i] >= x
  size_t lower_bound(uint64_t x) const {
    if (x >= bits_.size()) return size();
    return bits_.rank(x, 1);
  }

  size_t byteSize() const {
    return sizeof(*this) + bits_.byteSize();
  }
  size_t sampleSize() const {
    return 0;
  }
 private:
  SparseBitVector bits_;
  size_t size_;
};

