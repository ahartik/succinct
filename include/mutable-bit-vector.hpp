#pragma once
#include "bit-iterator.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <iostream>

class MutableBitVector {
 public:
  typedef BitIterator iterator;
  typedef ConstBitIterator const_iterator;
  MutableBitVector() {
    size_ = 0;
    resize(0);
  }
  explicit MutableBitVector(size_t n, bool val = false) {
    size_ = 0;
    resize(n, val);
  }
  MutableBitVector(const MutableBitVector& o) = default;
  MutableBitVector(MutableBitVector&& o)
      : bits_(std::move(o.bits_)), size_(o.size_) {
    o.size_ = 0;
  }
  MutableBitVector(std::initializer_list<bool> v) {
    size_ = 0;
    resize(v.size());
    size_t i = 0;
    for (bool b : v) {
      (*this)[i++] = b;
    }
  }

  void reserve(size_t n) {
    bits_.reserve((n + WordBits - 1) / WordBits);
  }
  void trim() {
    bits_ = std::vector<unsigned long>(bits_);
  }

  size_t size() const {
    return size_;
  }
  
  void resize(size_t n, bool val = false) {
    unsigned long fill = 0;
    if (n > size_) {
      int offset = size_ % WordBits;
      if (val) {
        fill = ~0UL;
      }
      if (offset != 0) {
        assert(bits_.size() != 0);
        if (val) {
          bits_.back() |= ~0ul << offset;
        } else {
          bits_.back() &= ~(~0ul << offset);
        }
      }
    }
    bits_.resize((n + WordBits - 1) / WordBits, fill);
    size_ = n;
    // set remaining bits to zero
    int offset = size_ % WordBits;
    if (!bits_.empty() && offset != 0) {
      bits_.back() &= ~(~0ul << offset);
    }
  }

  void pop_back() {
    resize(size_ - 1);
  }
  void push_back(bool b) {
    resize(size_ + 1);
    back() = b;
  }


  bool back() const {
    return (*this)[size_ - 1];
  }
  BitReference back() {
    return (*this)[size_ - 1];
  }

  BitIterator begin() {
    return BitIterator((*this)[0]);
  }

  BitIterator end() {
    return BitIterator((*this)[size_]);
  }

  ConstBitIterator begin() const {
    ConstBitReference ret;
    ret.data = &bits_[0];
    ret.offset = 0;
    return ConstBitIterator(ret);
  }

  ConstBitIterator end() const {
    ConstBitReference ret;
    ret.data = &bits_[size_ / WordBits];
    ret.offset = size_ % WordBits;
    return ConstBitIterator(ret);
  }

  template<typename Iterator>
  void insert(BitIterator pos, Iterator b, Iterator e) {
    size_t s = e - b;
    assert (b <= e);
    if (s == 0) return;
    size_t p = pos - begin();
    resize(size() + s);
    BitIterator read = end() - s - 1;
    for (size_t i = size() - 1; i >= p + s; --i) {
      assert(false);
      (*this)[i] = (bool)*read;
      read--;
    }
    Iterator it = b;
    for (size_t i = p; it != e; ++i) {
      (*this)[i] = (bool)*it;
      ++it;
    }
  }

  BitReference operator[](size_t pos) {
    BitReference ret;
    ret.data = &bits_[pos / WordBits];
    ret.offset = pos % WordBits;
    return ret;
  }

  bool operator[](size_t pos) const {
    size_t i = pos / WordBits;
    int offset = pos % WordBits;
    return (bits_[i] >> offset) & 1;
  }

  const unsigned long* data() const {
    return bits_.data();
  }
  size_t byteSize() const {
    return sizeof(*this) + bits_.size() * WordBits / 8;
  }
 private:
  std::vector<unsigned long> bits_;
  size_t size_;
};
