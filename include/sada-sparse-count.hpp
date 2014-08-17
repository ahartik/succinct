#pragma once
#include "suffix-array.hpp"
#include "sparse-int-array.hpp"
#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "rmq.hpp"
#include <vector>
#include <unordered_map>
#include <map>

template<bool OneOpt = true>
class SadaSparseCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  SadaSparseCount(const SuffixArray& sa) {
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

    SparseIntArray<int> counts(sa.size());
    int one_count = 0;
    int rest_count = 0;
    for (int i = 0; i < sa.size(); ++i) {
      int d = da.rank(sa.sa(i), 1);
      // assert(d < lens.size() || i == 0);
      if (prev[d] == -1) {
        prev[d] = i;
      } else {
        int r = lcp_rmq.rmq(prev[d]+1, i+1);
#if 0
        // Experiment to only have one position for every "real" internal node.
        // Improves size a bit when not using OneOpt.
        {
          int lc = sa.lcp(r);
          while (r > 0 && sa.lcp(r-1) == lc) r--;
        }
#endif
        int c = ++counts[r];
        if (c == 1) one_count++;
        if (c == 2) {
          one_count--;
          rest_count ++;
        }
        prev[d] = i;
      }
    }

    vector<int> count_pos;
    vector<int> one_pos;
//    count_pos.reserve(rest_count);
//    one_pos.reserve(one_count);
    for (size_t i = 0; i < counts.size(); ++i) {
      int c = counts.get(i);
      if (c == 0) continue;
      if (c == 1 && OneOpt) {
        one_pos.push_back(i);
      } else {
        count_pos.push_back(i);
      }
    }

    std::sort(count_pos.begin(), count_pos.end());
    std::sort(one_pos.begin(), one_pos.end());
    pos_ = SparseBitVector(count_pos.begin(),
                           count_pos.end());

    one_ = SparseBitVector(one_pos.begin(), one_pos.end());
 
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
  SparseBitVector one_;
  // pos_[i] == 1 <=> count[i] > 1
  SparseBitVector pos_;
  // unary encoding of count values.
  SparseBitVector count_;
};
