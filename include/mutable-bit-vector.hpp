#pragma once
#include "bit-iterator.hpp"
#include "bit-utils.hpp"

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
  const MutableBitVector& operator=(const MutableBitVector& o) {
    size_ = o.size_;
    bits_ = o.bits_;
    return *this;
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
    bits_ = std::vector<Word>(bits_);
  }

  size_t size() const {
    return size_;
  }
  
  void resize(size_t n, bool val = false) {
    Word fill = 0;
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

  template<bool Bit>
  BitPosIterator<MutableBitVector, Bit> bitPosBegin() const {
    return BitPosIterator<MutableBitVector, Bit>(
        BitPosIteratorAdaptor<MutableBitVector, Bit>(this));
  }

  template<bool Bit>
  BitPosIterator<MutableBitVector, Bit> bitPosEnd() const {
    size_t popcount = 0;
    for (Word w : bits_) {
      popcount += WordPopCount(w);
    }
    return BitPosIterator<MutableBitVector, Bit>(
        BitPosIteratorAdaptor<MutableBitVector, Bit>(popcount));
  }

  Word getWord(size_t i, int len) const {
    size_t pos = i / WordBits;
    size_t off = i % WordBits;
    Word mask =  (1LL << len) - 1;
    if (len == WordBits) mask = ~Word(0);
    Word lr = bits_[pos];
    Word hr = pos == bits_.size() - 1 ? 0 : bits_[pos + 1];
    Word l = lr >> off;
    Word h = off == 0 ? 0 : hr << (WordBits - off);
    return (h + l) & mask;
  }

  void setWord(size_t i, Word val, int len) {
    assert (i + len <= size());
    size_t pos = i / WordBits;
    size_t off = i % WordBits;
    Word mask =  (1LL << len) - 1;
    if (len == WordBits) mask = ~Word(0);
    Word fake_high;
    Word& lr = bits_[pos];
    Word& hr = off == 0 ? fake_high : 
        (pos < bits_.size() - 1 ? bits_[pos + 1] : fake_high);
    // set bits to zero before ORing
    lr &= ~(mask << off);
    hr &= ~(mask >> (WordBits - off));
    lr |= (val << off);
    hr |= (val >> (WordBits - off));
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

  const Word* data() const {
    return bits_.data();
  }

  Word* data() {
    return bits_.data();
  }

  size_t byteSize() const {
    return bits_.size() * sizeof(Word) +
           sizeof(size_);
  }
 private:
  std::vector<Word> bits_;
  size_t size_;
};

template<bool Bit>
class BitPosIteratorAdaptor<MutableBitVector, Bit> {
 public:
  BitPosIteratorAdaptor(size_t popcount) {
    pos_ = popcount;
    end_ = true;
  }
  BitPosIteratorAdaptor(const MutableBitVector* bv) {
    bv_ = bv;
    val_ = 0;
    pos_ = 0;
    end_ = false;
    findNext();
  }
  size_t pos() const {
    return pos_;
  }
  size_t get() const {
    return val_;
  }
  void inc() {
    ++val_;
    findNext();
    ++pos_;
  }
  void dec() {
    --val_;
    findPrev();
    --pos_;
  }
  const MutableBitVector* vec() const {
    return bv_;
  }
  bool end() const {
    return end_;
  }
 private:
  // moves val to next one
  void findNext() {
    for (; val_ < bv_->size() && (*bv_)[val_] == false; ++val_);
    if (val_ == bv_->size()) end_ = true;
  }
  void findPrev() {
    end_ = false;
    for (; val_ > 0 && (*bv_)[val_] == false; --val_);
    assert((*bv_)[val_]);
  }
  const MutableBitVector* bv_;
  size_t pos_;
  size_t val_;
  bool end_;
};
