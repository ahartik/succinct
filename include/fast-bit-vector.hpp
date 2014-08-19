#ifndef FASTBITVECTOR_H
#define FASTBITVECTOR_H // Small and simple bitvector.
// Designed to be fast and compact.
// Supports select and rank queries.
#include "mutable-bit-vector.hpp"
#include "bit-utils.hpp"

#include <cstddef>
#include <vector>
#include <cassert>
#include <stdint.h>

using std::size_t;

class FastBitVector {
  // These two CANNOT be modified.
  static const unsigned RankSample = 2048;
  static const unsigned RankSubSample = 384;
  // This CAN be tweaked.
  static const unsigned SelectSample = 2 * 2048;
  static const unsigned WordBits = 8 * sizeof(long);
 public:
  // Empty constructor.
  FastBitVector();
  explicit FastBitVector(const MutableBitVector& bv);
  explicit FastBitVector(const MutableBitVector&& bv);
  FastBitVector(const FastBitVector& other) : FastBitVector(other.bv_) { }
  FastBitVector(FastBitVector&& other);
  FastBitVector& operator=(FastBitVector&& other);
  FastBitVector& operator=(const FastBitVector& other);

  bool operator[](size_t pos) const {
    return bv_[pos];
  }

  // Number of positions < pos set with bit_value.
  size_t rank(size_t pos, bool bit_value) const {
    if (pos == 0) return 0;
    if (pos == size()) return count(bit_value);
    size_t block = pos / RankSample;
    unsigned remaining = pos % RankSample;
    size_t sum = rank_samples_[block].abs;
    int sub_block = remaining / RankSubSample;
    sum += subBlockRank(block, sub_block);
    remaining -= RankSubSample * sub_block;
    size_t word = (block * RankSample + sub_block * RankSubSample) / WordBits;
    size_t end_word = pos / WordBits;
    const Word *bits = bv_.data();
    for (; word < end_word; ++word) {
      sum += WordPopCount(bits[word]);
      remaining -= WordBits;
    }
    // Add first bits from the last word.
    unsigned long mask = (1LL << remaining) - 1LL;
    sum += WordPopCount(bits[word] & mask);
    if (bit_value == 0) return pos - sum;
    return sum;
  }

  // Returns smallest position pos so that rank(pos,bit) == idx
  size_t select(size_t idx, bool bit) const {
    if (idx == 0) return 0;
    assert(idx <= count(bit));
    // Start from sampling.
    size_t block = idx / SelectSample;
    size_t left = select_samples_[bit][block];
    size_t right = select_samples_[bit][block + 1];
    // Binary search correct rank-sample
    while (left + 1 < right) {
      size_t c = (left + right) / 2;
      size_t r = bit ? rank_samples_[c].abs : c * RankSample - rank_samples_[c].abs;
      if (r >= idx) {
        right = c;
      } else {
        left = c;
      }
    }
    size_t word_start = left * RankSample / WordBits;
    size_t word_rank = bit ? rank_samples_[left].abs : left * RankSample - rank_samples_[left].abs;
    assert(word_rank <= idx);
    // Linear search for correct rank-sub-sample
    const size_t total_words = (size() + WordBits - 1) / WordBits;
    for (int sub_block = 5; sub_block > 0; --sub_block) {
      size_t r = subBlockRank(left, sub_block);
      r = bit ? r : (sub_block * RankSubSample - r);
      if (r + word_rank < idx) {
        size_t new_start = word_start + sub_block * RankSubSample / WordBits;
        if (new_start < total_words) {
          word_start = new_start;
          word_rank += r;
          break;
        }
      }
    }
    assert(word_rank <= idx);

    const Word* bits = bv_.data();
    // Scan words
    for (size_t w = word_start; w < total_words; ++w) {
      size_t pop = __builtin_popcountll(bits[w]);
      size_t r = word_rank + (bit ? pop : WordBits - pop);
      if (r >= idx) break;
      word_rank = r;
      word_start = w + 1;
    }

    // Scan bits
    size_t id = idx - word_rank;
    assert(id >= 0 && id <= WordBits);
    if (id == 0) return word_start * WordBits;
    size_t w = bits[word_start];
    if (!bit) w = ~w;
    return word_start * WordBits + WordSelect(w, id);
  }

  size_t size() const {
    return bv_.size();
  }
  size_t count(bool bit) const {
    if (bit) return popcount_;
    return size() - popcount_;
  }
  size_t extra_bits() const;
  size_t byteSize() const {
    return bv_.byteSize() + extra_bits() / 8;
  }

  ~FastBitVector() {
  }
  friend void swap(FastBitVector& a, FastBitVector& b);
 private:
  size_t subBlockRank(size_t block, int sub_block) const {
      return (rank_samples_[block].rel >> (11ull * (5 - sub_block))) & 0x7FFull;
  }
  void init();

  MutableBitVector bv_;
  size_t popcount_;
  struct RankBlock {
    uint64_t abs;
    uint64_t rel;
  };

  std::vector<RankBlock> rank_samples_;
  // uint32_t is enough for 2048 * 2^32 bits = 1TB
  // Should be good enough for few years.
  std::vector<uint32_t> select_samples_[2];
};

#endif
