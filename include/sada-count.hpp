#pragma once
#include "suffix-array.hpp"
#include "sparse-int-array.hpp"
#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "rmq.hpp"
#include <vector>

template<typename BitVector = FastBitVector>
class SadaCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  SadaCount(const SuffixArray& sa) {
    using std::vector;
    vector<Index> ends;
    const unsigned char* text =
        reinterpret_cast<const unsigned char*>(sa.text());
    for (Index i = 0; i < sa.size(); ++i) {
      if (text[i] <= 1) {
        ends.push_back(i);
      }
    }
    SparseBitVector da(ends.begin(), ends.end());
    RMQ<Index> lcp_rmq(sa.lcp_data(), sa.size());

    vector<int> prev(ends.size() + 1, -1);
    // vector<int> counts(sa.size(), 0);
    SparseIntArray<int> counts(sa.size());
    for (int i = 0; i < sa.size(); ++i) {
      int d = da.rank(sa.sa(i), 1);
      if (prev[d] == -1) {
        prev[d] = i;
      } else {
        int r = lcp_rmq.rmq(prev[d]+1, i+1);
        counts[r]++;
        prev[d] = i;
      }
    }
    int bit_count = 0;
    MutableBitVector bv(2 * sa.size());
    for (size_t i = 0; i < counts.size(); ++i) {
      for (int j = 0; j < counts.get(i); ++j) bv[bit_count++] = 0;
      bv[bit_count++] = 1;
    }
    bv.resize(bit_count + 1);
    bv_ = BitVector(bv);
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    if (range.first == range.second) return 0;
    size_t a = bv_.select(range.first + 1, 1) - range.first;
    size_t b = bv_.select(range.second, 1) - range.second +1;
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
  BitVector bv_;
};
