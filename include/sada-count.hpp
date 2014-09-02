#pragma once
#include "suffix-array.hpp"
#include "sparse-int-array.hpp"
#include "sada-count-common.hpp"
#include <vector>

template<typename BitVector = FastBitVector, bool UseFilter = false, typename Filter=SparseBitVector>
class SadaCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  SadaCount(const SuffixArray& sa) {
    SparseIntArray<int> counts(sa.size());
    SadaConstruct(sa, &counts);
    MutableBitVector bv(2 * sa.size());
    MutableBitVector filter_vec;
    if (UseFilter) {
      filter_vec.resize(sa.size());
    }
    int bit_count = 0;
    for (size_t i = 0; i < counts.size(); ++i) {
      size_t count = counts.get(i);
      if (UseFilter) {
        if (count == 1) {
          filter_vec[i] = 1;
          count = 0;
        }
      }
      for (int j = 0; j < count; ++j) bit_count++;
      bv[bit_count++] = 1;
    }
    bv.resize(bit_count + 1);
    bv_ = BitVector(bv);
    filter_ = Filter(filter_vec);
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    // This case does not work well with scount
    if (range.first == range.second) return 0;
    
    size_t a = scount(range.first + 1);
    size_t b = scount(range.second);
    size_t dup = b - a;
    size_t len = range.second - range.first;
    return len - dup;
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    return bv_.byteSize();
  }

 private:
  size_t scount(size_t i) const {
    size_t c = bv_.select(i, 1) - (i - 1);
    if (UseFilter) {
      c += filter_.rank(i, 1);
    }
    return c;
  }
  BitVector bv_;
  Filter filter_;
};
