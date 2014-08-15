#ifndef BALANCED_WAVELET_H
#define BALANCED_WAVELET_H

#include "fast-bit-vector.hpp"
#include <stdint.h>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <cmath>

template<typename BitVector = FastBitVector>
class BalancedWavelet;

class BalancedWaveletEncoder {
  struct ConstructNode {
    MutableBitVector vec;
    ConstructNode *child[2] = {nullptr, nullptr};
    ~ConstructNode() {
      delete child[0];
      delete child[1];
    }
  };
 public:
  template <typename It>
  BalancedWaveletEncoder(It begin, It end, int b) : bits(b) {
    for (It it = begin; it != end; ++it) {
      append(*it);
    }
  }

  template <typename It>
  BalancedWaveletEncoder(It begin, It end) {
    uint64_t max = 0;
    for (It it = begin; it != end; ++it) {
      if (uint64_t(*it) > max) {
        max = *it;
      }
    }
    bits = 1 + log2(max);
    for (It it = begin; it != end; ++it) {
      append(*it);
    }
  }

  explicit BalancedWaveletEncoder(int b) : bits(b) { }

  void append(uint64_t value) {
    assert (value < (1ULL<<bits));
    ConstructNode* add = &croot;
    int j = 0;
    for (int i = bits - 1; i >= 0; --i) {
      bool b = (value >> i) & 1;
      if (add->child[b] == nullptr) {
        add->child[b] = new ConstructNode;
      }
      add->vec.push_back(b);
      add = add->child[b];
      j++;
    }
  }

 private:
  template<typename T>
  friend class BalancedWavelet;
  int bits;
  ConstructNode croot;
};

template<typename BitVector>
class BalancedWavelet {
 public:
  static const size_t npos = -1;
  BalancedWavelet(BalancedWaveletEncoder&& enc) 
      : size_(enc.croot.vec.size()),
        bits_(enc.bits) {
    MutableBitVector init;
    init.reserve(bits_ * enc.croot.vec.size());

    std::vector<BalancedWaveletEncoder::ConstructNode*> q;
    q.push_back(&enc.croot);
    for (size_t i = 0; i < q.size(); ++i) {
      auto* n = q[i];
      init.insert(init.end(), n->vec.begin(), n->vec.end());
      // Clear the vector to free memory.
      n->vec.trim();
      for (int j = 0; j < 2; ++j)
        if (n->child[j] != nullptr)
          q.push_back(n->child[j]);
    }
    delete enc.croot.child[0];
    delete enc.croot.child[1];
    enc.croot.child[0] = nullptr;
    enc.croot.child[1] = nullptr;
    BitVector tree(init);
    swap(tree_, tree);
  }

  template<typename IntType>
  BalancedWavelet(std::vector<IntType> && vec)
      : BalancedWavelet(&vec[0], vec.size()) { }

  template<typename IntType>
  BalancedWavelet(IntType* vec, size_t size) 
      : size_(size) {
    if (size == 0) return;
    IntType max = *std::max_element(&vec[0], &vec[size]);
    bits_ = 1 + log2(max);
    if (bits_ <= 0) bits_ = 1;
    MutableBitVector init;
    init.reserve(bits_ * size_);
    uint64_t mask = 0ull;

    for (int b = 0; b < bits_; ++b) {
      uint64_t bit = 1ull << (bits_ - b - 1);
      for (size_t i = 0; i < size_; ++i) {
        init.push_back((vec[i] & bit) != 0);
      }
      size_t start = 0;
      auto part_pred = [&](IntType x) -> bool {
        return (x & bit) == 0;
      };
      if (bit == 1) break;
      // sort each block
      for (size_t i = 0; i <= size_; ++i) {
        if (i == size_ || (vec[i] & mask) != (vec[start] & mask)) {
          std::stable_partition(&vec[start], &vec[i], part_pred);
          start = i;
        }
      }
      mask |= bit;
    }
    BitVector tree(init);
    swap(tree_, tree);
  }
  
#define VEC_INIT

  template<typename It>
  BalancedWavelet(It begin, It end, int bits) 
#ifdef VEC_INIT
       : BalancedWavelet(std::vector<
                         typename std::iterator_traits<It>::value_type
                         >(begin, end)) {
#else 
       : BalancedWavelet(BalancedWaveletEncoder(begin, end, bits)) {
#endif
  }

  template<typename It>
  BalancedWavelet(It begin, It end) 
#ifdef VEC_INIT
       : BalancedWavelet(std::vector<
                         typename std::iterator_traits<It>::value_type
                         >(begin, end)) {
#else 
       : BalancedWavelet(BalancedWaveletEncoder(begin, end)) {
#endif
  }

  BalancedWavelet(const BalancedWavelet& o) = delete;

  BalancedWavelet() {
    size_ = 0;
    bits_ = 0;
  }
  BalancedWavelet(BalancedWavelet&& o) 
      : tree_(std::move(o.tree_)), size_(o.size_), bits_(o.bits_) { }

  const BalancedWavelet& operator=(BalancedWavelet&& o) {
    tree_ = std::move(o.tree_);
    size_ = o.size_;
    bits_ = o.bits_;
    o.size_ = 0;
    o.bits_ = 0;
    return *this;
  }

  class Iterator {
   public:
    Iterator(const BalancedWavelet& wt)
        : high_bits(0),
          len(wt.size_),
          offset(0),
          bit(wt.bits_ - 1),
          begin_rank(0),
          end_rank(wt.tree_.rank(wt.size_, 1)),
          level_skip(wt.size_),
          vec(&wt.tree_)
    {}
    // Null constructor - only operator= is supported.
    Iterator() 
        : high_bits(0),
          len(0),
          offset(0),
          bit(0),
          begin_rank(0),
          end_rank(0),
          level_skip(0),
          vec(nullptr)
    {}

    const Iterator& operator=(const Iterator& o) {
      high_bits = o.high_bits;
      len = o.len;
      offset = o.offset;
      bit = o.bit;
      begin_rank = o.begin_rank;
      end_rank = o.end_rank;
      level_skip = o.level_skip;
      vec = o.vec;
      return *this;
    }

    uint64_t splitValue() const {
      return high_bits + (1LL << bit);
    }

    bool isLeaf() const {
      return bit == 0;
    }

    Iterator child(bool right) const {
      return Iterator(*this, right);
    }

    bool operator[](size_t i) const {
      return (*vec)[offset + i];
    }

    size_t rank(size_t pos, bool bit) const {
      assert(pos <= len);
      size_t orank = bit ? begin_rank : offset - begin_rank;
      return vec->rank(offset + pos, bit) - orank;
    }
    size_t select(size_t idx, bool bit) const {
      size_t orank = bit ? begin_rank : offset - begin_rank;
      return vec->select(idx + orank, bit) - offset;
    }
    size_t count() const {
      return len;
    }
   private:
    Iterator(const Iterator& parent, bool right) : vec(parent.vec) {
      bit = parent.bit - 1;
      level_skip = parent.level_skip;
      if (right) {
        offset = parent.offset + level_skip + (parent.len - parent.end_rank);
        len = parent.end_rank;
        high_bits = parent.high_bits + (1LL << parent.bit);
      } else {
        offset = parent.offset + level_skip;
        high_bits = parent.high_bits;
        len = parent.len - parent.end_rank;
      }
      begin_rank = vec->rank(offset, 1);
      end_rank = vec->rank(offset + len, 1) - begin_rank;
    }
    uint64_t high_bits;
    size_t len;
    size_t offset;
    size_t bit;
    size_t begin_rank;
    size_t end_rank;
    size_t level_skip;
    const BitVector* vec;
    friend class BalancedWavelet;
  };

  size_t rank(size_t pos, uint64_t value) const {
    assert(pos <= size());
    Iterator it(*this);
    for (;;) {
      bool bit = value >= it.splitValue();
      pos = it.rank(pos, bit);
      if (it.isLeaf()) break;
      it = it.child(bit);
    }
    return pos;
  }

  size_t rankLE(size_t pos, uint64_t value) const {
    assert(pos <= size());
    size_t ret = 0;
    Iterator it(*this);
    for (;;) {
      bool bit = value >= it.splitValue();
      size_t np = it.rank(pos, bit);
      if (bit) {
        ret += pos - np;
      }
      pos = np;
      if (it.isLeaf()) break;
      else it = it.child(bit);
    }
    return pos + ret;
  }

  uint64_t operator[](size_t i) const {
    Iterator it(*this);
    while (!it.isLeaf()) {
      bool b = it[i];
      Iterator nit = it.child(b);
      i = it.rank(i, b);
      it = nit;
    }
    return it.high_bits + it[i];
  }

  size_t select(size_t rank, uint64_t value) const {
    return select(Iterator(*this), rank, value);
  }

  size_t select(Iterator it, size_t rank, uint64_t value) const {
    if (it.count() == 0) return 0;
    if (rank == 0) return 0;
    bool b = value >= it.splitValue();
    if (it.isLeaf()) {
      return it.select(rank, b);
    }
    rank = select(it.child(b), rank, value);
    return it.select(rank, b);
  }

  size_t size() const {
    return size_;
  }
  size_t bitSize() const {
    return tree_.bitSize() + sizeof(*this) * 8;
  }
 private:

  BitVector tree_;
  size_t size_;
  int bits_;
};

#endif
