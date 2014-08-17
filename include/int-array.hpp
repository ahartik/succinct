#pragma once
#include "mutable-bit-vector.hpp"

#include <cstdint>
#include <vector>
#include <cassert>
#include <cstddef>

#include <iostream>

class IntArray {
 public:
  IntArray() : width_(1), size_(0) {
  }
  IntArray(int width, size_t size) 
      : width_(width), size_(size), bv_(size * width)
  { }
  IntArray(IntArray&& o) 
    : width_(o.width_),
      size_(o.size_)
  {
    bv_ = std::move(o.bv_);
    o.size_ = 0;
  }

  IntArray(const IntArray& o) = default;
  IntArray& operator=(const IntArray& o) = default;

  IntArray& operator=(IntArray&& o) {
    bv_ = std::move(o.bv_);
    size_ = o.size_;
    width_ = o.width_;
    o.size_ = 0;
    return *this;
  }

  size_t size() const {
    return size_;
  }
  int width() const {
    return width_;
  }
  Word maxValue() const {
    if (width_ == WordBits) return Word(-1);
    return (1ull << width_) - 1;
  }
  Word get(size_t i) const {
    return bv_.getWord(i * width_, width_);
  }

  void set(size_t i, Word val) {
    bv_.setWord(i * width_, val, width_);
  }
  size_t byteSize() const {
    return sizeof(*this) + bv_.byteSize() - sizeof(bv_);
  }
 private:
  int width_;
  size_t size_;
  MutableBitVector bv_;
  // std::vector<Word> vec_;
};
