#pragma once
#include <vector>
#include "fast-bit-vector.hpp"
#include <iostream>
#include <algorithm>

static uint64_t len_counter[65] = {};

class DeltaEncoder {
 public:
  DeltaEncoder(std::vector<uint64_t>* b) {
    offset = 0;
    bits = b;
    finished = false;
  }
  ~DeltaEncoder() {
    if (!finished)
      finish();
  }
  void finish() {
    bits->push_back(0);
    finished = true;
  }
  void add(uint64_t x) {
    uint64_t code = 0;
    int shift = 0;
    uint64_t len = 1 + WordLog2(x);
    uint64_t len2 = WordLog2(len);
    code = x ^ (1ll << (len - 1));
    shift = len - 1;
    code |= len << shift;
    shift += len2 * 2 + 1;
    writeCode(code, shift);
  }

  uint64_t tell() {
    return bits->size() * 64 - offset;
  }

 private:
  void writeBit(bool b) {
    if (offset == 0) {
      bits->emplace_back(0);
      offset = 64;
    }
    offset--;
    bits->back() |= (uint64_t(b) << uint64_t(offset));
  }
  void writeCode(uint64_t c, int len) {
#if 1
    len_counter[len]++;
    if (offset == 0) {
      bits->emplace_back(0);
      offset = 64;
    }

    if (offset >= len) {
      bits->back() |= (c << (offset - len));
      offset -= len;
    } else {
      bits->back() |= (c >> (len - offset));
      bits->push_back(0);
      bits->back() |= (c << (64 + offset - len));
      offset -= len;
      offset += 64;
    }
#else
    // simple and slow
    for (int i = len-1; i >= 0; --i) {
      writeBit((c >> i) & 1);
    }
#endif
  }

  int offset;
  std::vector<uint64_t>* bits;
  bool finished;
};

class DeltaReader {
 public:
  DeltaReader(const uint64_t* b, uint64_t pos) {
    bits = b + pos / 64;
    offset = pos % 64;
  }
  uint64_t read() {
    uint64_t c = readCode();
    int ll = __builtin_clzll(c);
    ll = ll * 2 + 1;
    uint64_t len = c >> (64 - ll);
    uint64_t x = 1ull << (len-1);
    uint64_t rest_mask = ((1ull << (len - 1)) - 1);

    int code_len = ll + len - 1;

    x |= (c>>(64 - code_len)) & rest_mask;

    offset += code_len;
    if (offset >= 64) {
      offset -= 64;
      bits++;
    }
    return x;
  }
 private:
  uint64_t readCode() {
    if (offset == 0) return bits[0];
    uint64_t r = (bits[0]) << (offset);
    r |= (bits[1]) >> (64-offset);
    return r;
  }
  bool readBit() {
    bool r = ((*bits) >> (uint64_t)offset) & 1;
    offset++;
    if (offset == 64) {
      bits++;
      offset = 0;
    }
    return r;
  }
  const uint64_t* bits;
  int offset;
};
template<size_t BlockSize = 64>
class DeltaVector {
 public:
  static constexpr uint64_t BLOCK_SIZE = BlockSize;
  DeltaVector() {
    size_ = 0;
  }
  template<typename IntT>
  DeltaVector(const std::vector<IntT>& vec) {
    DeltaEncoder enc(&bits_);
    IntT last = 0;
    size_ = vec.size();
    int num_blocks = (size_ + BLOCK_SIZE - 1) / BLOCK_SIZE;
    block_val_.resize(num_blocks);
    block_pos_.resize(num_blocks);
    for (size_t i = 0; i < vec.size(); ++i) {
      size_t block = i / BLOCK_SIZE;
      if (i % BLOCK_SIZE == 0) {
        block_pos_[block] = enc.tell();
        block_val_[block] = vec[i];
      } else {
        assert(vec[i] > last);
        uint64_t d = vec[i] - last;
        enc.add(d);
      }
      last = vec[i];
    }
    enc.finish();
    // trim vector
    bits_ = std::vector<uint64_t>(bits_);
  }

  DeltaVector(const DeltaVector& v) = default;
  DeltaVector(DeltaVector&& v)
      : bits_(std::move(v.bits_)),
        block_pos_(std::move(v.block_pos_)),
        block_val_(std::move(v.block_val_)),
        size_(v.size())
  {
    v.size_ = 0;
  }

  DeltaVector& operator=(const DeltaVector& o) {
    bits_ = o.bits_;
    block_pos_ = o.block_pos_;
    block_val_ = o.block_val_;
    size_ = o.size_;
    return *this;
  }

  DeltaVector& operator=(DeltaVector&& o) {
    bits_ = std::move(o.bits_);
    block_pos_ = std::move(o.block_pos_);
    block_val_ = std::move(o.block_val_);
    size_ = o.size_;
    o.size_ = 0;
    return *this;
  }

  size_t size() const {
    return size_;
  }

  uint64_t operator[](size_t i) const {
    size_t block = i / BLOCK_SIZE;
    size_t offset = i % BLOCK_SIZE;
    uint64_t x = block_val_[block];
    DeltaReader r(bits_.data(), block_pos_[block]);
    for (size_t j = offset; j > 0; --j) {
      x += r.read();
    }
    return x;
  }

  // Finds the first position i where (*this)[i] >= x
  size_t lower_bound(uint64_t x) const {
    size_t b = std::upper_bound(block_val_.begin(),
                                block_val_.end(),
                                x) - block_val_.begin();
    if (b == 0) return 0;
    b--;
    if (b * BLOCK_SIZE >= size()) {
      return size();
    }
    if (block_val_[b] == x) return BLOCK_SIZE * b;
    assert (block_val_[b] < x);
    DeltaReader r(bits_.data(), block_pos_[b]);
    uint64_t y = block_val_[b];
    for (size_t i = 1;i < BLOCK_SIZE; ++i) {
      y += r.read();
      if (y >= x) {
        return b * BLOCK_SIZE + i;
      }
    }
    return (b + 1) * BLOCK_SIZE;
  }

  size_t byteSize() const {
    return sizeof(*this) +
           (bits_.size() + block_pos_.size() + block_val_.size()) * 8;
  }
  size_t sampleSize() const {
    return (block_pos_.size() + block_val_.size()) * 8;
  }
 private:
  std::vector<uint64_t> bits_;
  std::vector<uint64_t> block_pos_;
  std::vector<uint64_t> block_val_;
  size_t size_;
};
