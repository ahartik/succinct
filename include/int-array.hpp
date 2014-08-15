#pragma once
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
      : width_(width), size_(size), vec_(2 + size * width / 64)
  { }
  IntArray(IntArray&& o) 
    : width_(o.width_),
      size_(o.size_)
  {
    vec_ = std::move(o.vec_);
    o.size_ = 0;
  }

  const IntArray& operator=(IntArray&& o) {
    vec_ = std::move(o.vec_);
    o.size_ = 0;
    width_ = o.width_;
    size_ = o.size_;
    return *this;
  }

  size_t size() const {
    return size_;
  }
  int width() const {
    return width_;
  }
  uint64_t maxValue() const {
    return (1ull << width_) - 1;
  }
  uint64_t get(size_t i) const {
    size_t pos = i * width_ / 64;
    size_t off = i * width_ % 64;
    uint64_t mask =  (1LL << width_) - 1;
    uint64_t lr = vec_[pos];
    uint64_t hr = vec_[pos + 1];
    uint64_t l = lr >> off;
    uint64_t h = off == 0 ? 0 : hr << (64 - off);
    return (h + l) & mask;
  }
  void set(size_t i, uint64_t val) {
    size_t pos = i * width_ / 64;
    size_t off = i * width_ % 64;
    uint64_t mask =  (1LL << width_) - 1;
    uint64_t fake_high;
    uint64_t& lr = vec_[pos];
    uint64_t& hr = off == 0 ? fake_high : vec_[pos + 1];
    // set bits to zero before ORing
    lr &= ~(mask << off);
    hr &= ~(mask >> (64 - off));
    lr |= (val << off);
    hr |= (val >> (64 - off));
  }
  size_t byteSize() const {
    return vec_.size() * sizeof(uint64_t) + sizeof(*this);
  }
 private:
  int width_;
  size_t size_;
  std::vector<uint64_t> vec_;
};
