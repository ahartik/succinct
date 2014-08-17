#pragma once
#include "suffix-array.hpp"

#include <vector>
#include <unordered_set>

class BruteCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  BruteCount(const SuffixArray& sa) {
    Index length = sa.size();
    da_.resize(length);
    std::vector<int> dn(length);
    int d = 0;
    const char* text = sa.text();
    for (Index i = 0; i < sa.size(); ++i) {
      dn[i] = d;
      if ((unsigned char)text[i] <= 1) {
        d++;
      }
    }
    for (Index i = 0; i < length; ++i) {
      da_[i] = dn[sa.sa(i)];
    }
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    std::unordered_set<Index> found;
    for (Index i = range.first; i < range.second; ++i) {
      found.insert(da_[i]);
    }
    return found.size();
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    return da_.size() * sizeof(int) + sizeof(*this);
  }

 private:
  std::vector<int> da_;
};
