#include "int-array.hpp"
#include "mutable-bit-vector.hpp"

#define RANK_SELECT_ATTRIB
// #define RANK_SELECT_ATTRIB __attribute__((noinline))

template<int BlockSize = 63>
class RRRBitVector {
 public:
  // Must be multiple of BlockSize
  static const size_t SuperBlockSize = BlockSize * 32;
  // Can be modified
  static const size_t SelectSample = SuperBlockSize * 2;
  RRRBitVector() {
    size_ = 0;
    for (int k = 0; k <= BlockSize; ++k) {
      Word possible = Binomial(k, BlockSize);
      k_len_[k] = WordLog2(possible) + 1;
    }
    popcount_ = 0;
  }
  RRRBitVector(const MutableBitVector& vec) {
    size_ = vec.size();
    for (int k = 0; k <= BlockSize; ++k) {
      Word possible = Binomial(k, BlockSize);
      k_len_[k] = WordLog2(possible) + 1;
    }
    popcount_ = 0;

    int bits_for_class = WordLog2(BlockSize) + 1;
    size_t blocks = (vec.size() + BlockSize - 1) / BlockSize;
    block_class_ = IntArray(bits_for_class, blocks);
    size_t super_blocks = (vec.size() + SuperBlockSize - 1) / SuperBlockSize;
    super_rank_.resize(super_blocks);
    super_pos_.resize(super_blocks);
    Word rank = 0;
    size_t count[2] = {0, 0};
    select_samples_[0].push_back(0);
    select_samples_[1].push_back(0);

    for (size_t i = 0; i < blocks; ++i) {
      Word w = vec.getWord(i * BlockSize, BlockSize);
      int k = WordPopCount(w);
      popcount_ += k;
      count[0] += BlockSize - k;
      count[1] += k;
      block_class_.set(i, k);
      Word c = encode(k, w);
      int l = kLen(k);
      // push back a word
      size_t s = idx_.size();
      idx_.resize(s + l);
      idx_.setWord(s, c, l);
      assert (idx_.getWord(s, l) == c);
      if (i % (SuperBlockSize / BlockSize) == 0) {
        size_t si  = i / (SuperBlockSize / BlockSize);
        super_rank_[si] = rank;
        super_pos_[si] = s;
      }
      rank += k;
      for (int b = 0; b < 2; ++b) {
        if (count[b] >= SelectSample) {
          count[b] -= SelectSample;
          select_samples_[b].push_back(i * BlockSize / SuperBlockSize);
        }
      }
    }
    idx_.trim();
  }
  RRRBitVector(RRRBitVector<BlockSize>&& o) {
    size_ = 0;
    for (int k = 0; k <= BlockSize; ++k) {
      Word possible = Binomial(k, BlockSize);
      k_len_[k] = WordLog2(possible) + 1;
    }
    popcount_ = 0;
    swap(*this, o);
  }

  RRRBitVector(const RRRBitVector<BlockSize>& o) = default;
  RRRBitVector& operator=(const RRRBitVector<BlockSize>& o) = default;

  bool operator[](size_t i) const {
    size_t target_block = i / BlockSize;
    size_t block_offset = i % BlockSize;
    size_t super_block = i / SuperBlockSize;
    Word pos = super_pos_[super_block];
    size_t block = super_block * SuperBlockSize / BlockSize;
    while (block < target_block) {
      int k = block_class_.get(block);
      pos += kLen(k);
      ++block;
    }
    assert(block == target_block);
    int k = block_class_.get(block);
    return decodeIndex(k, idx_.getWord(pos, kLen(k)), block_offset);
  }

  size_t rank(size_t i, bool bit) const RANK_SELECT_ATTRIB {
    if (i == size()) {
      return count(bit);
    }
    assert (i < size());
    size_t target_block = i / BlockSize;
    size_t block_offset = i % BlockSize;
    size_t super_block = i / SuperBlockSize;
    Word pos = super_pos_[super_block];
    size_t block = super_block * (SuperBlockSize / BlockSize);
    size_t rank = super_rank_[super_block];
    while (block < target_block) {
      int k = block_class_.get(block);
      rank += k;
      pos += kLen(k);
      ++block;
    }
    int k = block_class_.get(block);
    Word code = idx_.getWord(pos, kLen(k));
#if 1
    int r = decodeRank(k, code, block_offset);
    rank += r;
#else
    rank += code;
#endif
    return bit ? rank : i - rank;
  }

  size_t select(size_t x, bool bit) const  RANK_SELECT_ATTRIB {
    if (x == 0) return 0;
    size_t sample_idx = x / SelectSample;
    // Binary search super blocks:
    Word left_sb = select_samples_[bit][sample_idx];
    Word right_sb;
    if (sample_idx + 2 >= select_samples_[bit].size())
      right_sb = super_pos_.size();
    else
      right_sb = select_samples_[bit][sample_idx + 2];

    while (left_sb + 1 < right_sb) {
      size_t c = (left_sb + right_sb) / 2;
      size_t super_rank = super_rank_[c];
      size_t r = bit ? super_rank : c * SuperBlockSize - super_rank;
      if (r >= x) {
        right_sb = c;
      } else {
        left_sb = c;
      }
    }

    Word super_block = left_sb;
    Word block = super_block * (SuperBlockSize / BlockSize);
    size_t pos = super_pos_[super_block];
    size_t rank = super_rank_[super_block];
    size_t idx = block * BlockSize;
    if (!bit) rank = idx - rank;
    while (true) {
      assert(block < block_class_.size());
      assert(block < right_sb * (SuperBlockSize /  BlockSize));
      int k = block_class_.get(block);
      int r = bit ? k : BlockSize - k;
      if (rank + r >= x) {
        int d = decodeSelect(k, idx_.getWord(pos, kLen(k)), x - rank, bit);
        idx += d;
        return idx;
      }
      rank += r;
      pos += kLen(k);
      idx += BlockSize;
      ++block;
    }
  }
  size_t size() const {
    return size_;
  }
  size_t byteSize() const {
    return sizeof(*this) + idx_.byteSize() + block_class_.byteSize() +
        super_rank_.size() * sizeof(Word) +
        select_samples_[0].size() * sizeof(Word) +
        select_samples_[1].size() * sizeof(Word) +
        super_pos_.size() * sizeof(Word);
  }
  size_t count(bool bit) const {
    if (bit) return popcount_;
    return size() - popcount_;
  }
  friend void swap(RRRBitVector<BlockSize>& a, RRRBitVector<BlockSize> b) {
    using std::swap;
    swap(a.popcount_, b.popcount_);
    swap(a.select_samples_[0], b.select_samples_[0]);
    swap(a.select_samples_[1], b.select_samples_[1]);
    swap(a.super_pos_, b.super_pos_);
    swap(a.super_rank_, b.super_rank_);
    swap(a.block_class_, b.block_class_);
    swap(a.size_, b.size_);
    swap(a.idx_, b.idx_);
  }
 private:
  friend class RRRBitVectorTest;

  static bool decodeIndex(int k, Word code, int idx) {
    if (k == 0) return 0;
    if (k == BlockSize) return 1;
    bool ret = 0;
    for (int t = 0; t <= idx; ++t) {
      Word b = Binomial(k, BlockSize - t - 1);
      ret = 0;
      if (code >= b) {
        ret = 1;
        code -= b;
        k --;
      }
    }
    return ret;
  }
  static inline int decodeSelect(int k, Word code, int x, bool bit) {
    int rank = 0;
    if (x == 0) return 0;
    if (k == 0) return bit ? 0 : x;
    if (k == BlockSize) return bit ? x : 0;
    for (int t = 0; t < BlockSize; ++t) {
      // if (code == 0) break;
      Word b = Binomial(k, BlockSize - t - 1);
      if (code >= b) {
        rank ++;
        code -= b;
        k --;
      }
      int r = bit ? rank : t + 1 - rank;
      if (r >= x) return t + 1;
    }
    assert(false);
    return BlockSize;
  }
  static inline int decodeRank(int k, Word code, int idx) {
    int ret = 0;
    if (k == 0) return 0;
    if (k == BlockSize) return idx;
    for (int t = 0; t < idx; ++t) {
      // if (code == 0) break;
      // if (k == 0) break;
      Word b = Binomial(k, BlockSize - t - 1);
      if (code >= b) {
        ret ++;
        code -= b;
        k --;
      }
    }
    return ret;
  }
#if 0
  // For debugging purposes
  static Word decode(int k, Word code) {
    Word ret = 0;
    if (k == 0) return 0;
    if (k == BlockSize) return (1ull << BlockSize) - 1;
    for (int t = 0; t < BlockSize; ++t) {
      // if (code == 0) break;
      Word b = Binomial(k, BlockSize - t - 1);
      if (code >= b) {
        ret |= 1ull << t;
        code -= b;
        k --;
      }
    }
    return ret;
  }
#endif

  static Word encode(int k, Word x) {
    Word ret = 0;
    if (k == 0) return 0;
    if (k == BlockSize) return 0;
    for (int t = 0; t < BlockSize; ++t) {
      if (k == 0) break;
      if ((x >> t) & 1) {
        ret += Binomial(k, BlockSize - t - 1);
        k --;
      }
    }
    return ret;
  }
  int kLen(int k) const {
    return k_len_[k];
  }
  size_t popcount_;
  int k_len_[BlockSize + 1];
  std::vector<Word> select_samples_[2];
  std::vector<Word> super_pos_;
  std::vector<Word> super_rank_;
  IntArray block_class_;
  size_t size_;
  MutableBitVector idx_;
};
