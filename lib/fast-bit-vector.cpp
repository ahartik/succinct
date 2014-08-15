#include "fast-bit-vector.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>

FastBitVector::FastBitVector() {
  size_ = 0;
  popcount_ = 0;
  bits_ = nullptr;
  rank_samples_ = nullptr;
  select_samples_[0] = nullptr;
  select_samples_[1] = nullptr;
}

FastBitVector::FastBitVector(const MutableBitVector& data) {
  size_ = data.size();
  const size_t word_count = (WordBits + size_ - 1) / WordBits;
  bits_ = new unsigned long[word_count];
  // Init bits.
  popcount_ = 0;
  memcpy(bits_, data.data(), sizeof(long) * word_count);
  for (size_t i = 0; i < word_count; ++i) {
    popcount_ += __builtin_popcountll(bits_[i]);
  }

  // Init rank samples.
  rank_samples_ = new RankBlock[2 + size_ / RankSample];
  rank_samples_[0].abs = 0;
  size_t sum = 0;
  for (size_t i = 0; i <= size_/RankSample; i++) {
    uint64_t sub_block[6] = {};
    for (size_t j = 0; j < RankSample / WordBits; ++j) {
      size_t idx = i * RankSample / WordBits + j;
      if (idx >= word_count) break;
      int count = __builtin_popcountll (bits_[idx]);
      sum += count;
      sub_block[j * WordBits / RankSubSample] += count;
    }
    for (int j = 1; j < 6; ++j) {
      sub_block[j] += sub_block[j-1];
    }
    // Put in reverse order to remove branch for sub_block = 0
    rank_samples_[i].rel = 
        (sub_block[0] << 44) + 
        (sub_block[1] << 33) + 
        (sub_block[2] << 22) + 
        (sub_block[3] << 11) + 
        (sub_block[4] << 00);
    rank_samples_[1 + i].abs = sum;
  }

  // Init select samples.
  select_samples_[1] = new uint32_t[2 + popcount_ / SelectSample];
  select_samples_[0] = new uint32_t[2 + (size_ - popcount_) / SelectSample];
  size_t sums[2] = {0, 0};
  size_t idx[2] = {1, 1};
  select_samples_[0][0] = select_samples_[1][0] = 0;
  for (size_t i = 0; i < word_count; i++) {
    int count = __builtin_popcountll(bits_[i]);
    sums[1] += count;
    sums[0] += WordBits - count;
    for (int bit = 0; bit != 2; ++bit) {
      if (sums[bit] >= SelectSample) {
        sums[bit] -= SelectSample;
        size_t id = idx[bit]++;
        size_t rs = i * WordBits / RankSample;
        select_samples_[bit][id] = rs;
        if (bit)
          assert(rank_samples_[rs].abs <= id * SelectSample);
      }
    }
  }
  select_samples_[0][idx[0]] = 1 + (size_ / RankSample);
  select_samples_[1][idx[1]] = 1 + (size_ / RankSample);
}

FastBitVector::FastBitVector(FastBitVector&& other) 
    : size_(0),
      popcount_(0),
      bits_(nullptr),
      rank_samples_(nullptr),
      select_samples_{nullptr,nullptr} {
  swap(*this, other);
}

const FastBitVector& FastBitVector::operator=(FastBitVector&& other) {
  swap(*this, other);
  return *this;
}

// Returns smallest position pos so that rank(pos,bit) == idx

size_t FastBitVector::extra_bits() const {
  size_t r = 1 + popcount_ / SelectSample;
  r += 2 + (size_ - popcount_) / SelectSample;
  r += 2 + size_ / RankSample;
  return r * WordBits + sizeof(FastBitVector) * 8;
}

FastBitVector::~FastBitVector() {
  delete[] rank_samples_;
  delete[] select_samples_[0];
  delete[] select_samples_[1];
  delete[] bits_;
}
void swap(FastBitVector& a, FastBitVector& b) {
  using std::swap;
  swap(a.size_, b.size_);
  swap(a.popcount_, b.popcount_);
  swap(a.bits_, b.bits_);
  swap(a.rank_samples_, b.rank_samples_);
  swap(a.select_samples_[0], b.select_samples_[0]);
  swap(a.select_samples_[1], b.select_samples_[1]);
}
