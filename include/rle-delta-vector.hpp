#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include "delta-vector.hpp"
#include "sparse-bit-vector.hpp"

template<size_t BlockSize = 128>
class RLEDeltaVector {
 public:
  static constexpr uint64_t BLOCK_SIZE = BlockSize;
  template<typename IntT>
  RLEDeltaVector(const std::vector<IntT>& vec) {
    DeltaEncoder enc(&bits_);
    IntT last = 0;
    size_ = vec.size();
    std::vector<uint64_t> block_pos;
    std::vector<uint64_t> block_val;
    std::vector<uint64_t> block_idx;
  //   block_pos_.resize(num_blocks + 1);
  //   block_val_.resize(num_blocks + 1);
    size_t last_block = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
      if (i == 0 || enc.tell() - last_block >= BLOCK_SIZE) {
        last_block = enc.tell();
        block_idx.push_back(i);
        block_pos.push_back(enc.tell());
        block_val.push_back(vec[i]);
      } else {
        uint64_t d = vec[i] - last;
        enc.add(d);
        if (d == 1) {
          last = vec[i];
          ++i;
          int count = 1;
          for (;i < vec.size(); ++i) {
            uint64_t d = vec[i] - last;
            if (d != 1) {
              break;
            }
            ++count;
            last = vec[i];
          }
          --i;
          enc.add(count);
        }
      }
      last = vec[i];
    }
    block_idx.push_back(size());
    if (vec.empty()) {
      block_val.push_back(1);
    } else {
      block_val.push_back(vec.back() + 1);
    }
    block_pos.push_back(enc.tell());
    enc.finish();
   //  std::cout << bits_.size() * 8 << " bytes for encoding \n";
   //  std::cout << block_pos_.size() * 16 << " bytes for block samples \n";
    // trim vector
    bits_ = std::vector<uint64_t>(bits_);
    block_pos_ = SparseBitVector(block_pos.begin(), block_pos.end());
    block_val_ = SparseBitVector(block_val.begin(), block_val.end());
    block_idx_ = SparseBitVector(block_idx.begin(), block_idx.end());
  }

  RLEDeltaVector(const RLEDeltaVector& v) = default;
  RLEDeltaVector(RLEDeltaVector&& v)
      : bits_(std::move(v.bits_)),
        block_pos_(std::move(v.block_pos_)),
        block_val_(std::move(v.block_val_)),
        block_idx_(std::move(v.block_idx_)),
        size_(v.size())
  {
    v.size_ = 0;
  }

  size_t size() const {
    return size_;
  }

  uint64_t operator[](size_t i) const {
    size_t block = block_idx_.rank(i + 1, 1) - 1;
    size_t block_start = block_idx_.select1(block + 1) - 1;
    assert(block_start <= i);
    size_t offset = i - block_start;
    uint64_t pos = block_pos(block);
    uint64_t x = block_val(block);
    DeltaReader r(bits_.data(), pos);
    for (size_t j = offset; j > 0; --j) {
      uint64_t v = r.read();
      if (v == 1) {
        uint64_t c = r.read();
        if (c <= j) {
          x += c;
          j -= c - 1;
        } else {
          x += j;
          return x;
        }
      } else {
        x += v;
      }
    }
    return x;
  }

  // Finds the first position i where (*this)[i] >= x
  size_t lower_bound(uint64_t x) const {
    if (x >= block_val_.size()) return size();
    size_t b = block_val_.rank(x + 1, 1); // - 1;
     if (b == 0) return 0;
     b--;
    size_t block_start = block_idx_.select1(b + 1) - 1;
    if (block_start >= size()) {
      return size();
    }
    size_t block_pos = this->block_pos(b);
    uint64_t y = block_val(b);
    if (y == x) return block_start;
    assert (y < x);
    DeltaReader r(bits_.data(), block_pos);
    size_t next_block = block_idx_.select1(b + 2) - 1;
    assert(block_val(b + 1) >= x);
    for (int i = 1; block_start + i < next_block; ++i) {
      uint64_t z = r.read();
      if (z == 1) {
        int count = r.read();
        if (y + count >= x) {
          i += x - y - 1 ;
          return block_start + i;
        } else {
          y += count;
          i += count - 1;
        }
      } else {
        y += z;
      }
      if (y >= x) {
        return block_start + i;
      }
    }
    return next_block;
  }

  size_t byteSize() const {
    return sizeof(*this) +
           8 *bits_.size() + sampleSize();
           
  }
  size_t sampleSize() const {
    return (block_pos_.bitSize() + block_val_.bitSize() +
           block_idx_.bitSize()) / 8;
  }
 private:
  uint64_t block_pos(size_t i) const {
    return block_pos_.select1(i + 1) - 1;
  }
  uint64_t block_val(size_t i) const {
    return block_val_.select1(i + 1) - 1;
  }
  std::vector<uint64_t> bits_;
  SparseBitVector block_pos_;
  SparseBitVector block_val_;
  SparseBitVector block_idx_;
  size_t size_;
};
