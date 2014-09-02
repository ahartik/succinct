#pragma once
#include "suffix-array.hpp"
#include "sparse-int-array.hpp"
#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "rmq.hpp"
#include <vector>
#include <unordered_map>
#include <map>

template<bool OneOpt = true, typename OneVector = SparseBitVector>
class SadaSparseCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  SadaSparseCount(const SuffixArray& sa) {
    SparseIntArray<int> counts(sa.size());
    SadaConstruct(sa, &counts);

    using std::vector;
    vector<int> count_pos;
    MutableBitVector one(sa.size());
    for (size_t i = 0; i < counts.size(); ++i) {
      int c = counts.get(i);
      if (c == 0) continue;
      if (c == 1 && OneOpt) {
        one[i] = 1;
      } else {
        count_pos.push_back(i);
      }
    }

    pos_ = SparseBitVector(count_pos.begin(),
                           count_pos.end());

    one_ = OneVector(one);
    // Free some memory
    one = MutableBitVector();
 
    int c = 0;
    for (size_t i = 0; i < count_pos.size(); ++i) {
      c += counts[count_pos[i]];
      count_pos[i] = c;
    }
    count_ = SparseBitVector(count_pos.begin(),
                             count_pos.end());
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
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
    return pos_.byteSize() + count_.byteSize() + one_.byteSize();
  }

 private:
  int scount(int i) const {
    // How many counts?
    int c = pos_.rank(i, 1);
    // What is the total?
    int r = count_.select(c, 1);
    if (OneOpt) {
      // Add ones:
      r += one_.rank(i, 1);
    }
    return r;
  }

  // one_[i] == 1 <=> count[i] == 1
  OneVector one_;
  // pos_[i] == 1 <=> count[i] > 1
  SparseBitVector pos_;
  // unary encoding of count values.
  SparseBitVector count_;
};
