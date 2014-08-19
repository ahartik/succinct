#pragma once
#include "suffix-array.hpp"
#include "ilcp-common.hpp"

template<typename Wavelet>
class ILCPCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  ILCPCount(const SuffixArray& sa) {
    Index length = sa.size();
    std::vector<Index> ilcp(length);
    ILCPConstruct(sa, &ilcp);
    wt_ = Wavelet(ilcp.begin(), ilcp.end());
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    return wt_.rankLE(range.second, pattern.size() - 1) -
           wt_.rankLE(range.first, pattern.size() - 1);
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    return wt_.byteSize() + 1;
  }

 private:
  Wavelet wt_;
};
