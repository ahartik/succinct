#pragma once
#include "suffix-array.hpp"

template<typename Wavelet>
class ILCPCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  ILCPCount(const SuffixArray& sa) {
    Index length = sa.size();
    std::vector<Index> text_lcp(length);
    Index start = 0;
    int num_docs = 0;
    const char* text = sa.text();
    for (Index i = 0; i <= Index(length); ++i) {
      if ((unsigned char)text[i] <= 1 || i == Index(length)) {
        const char* doc = text + start;
        Index doc_len = i - start;
        SuffixArray doc_sa(doc, doc_len);
        for (Index j = 0; j < doc_len; ++j) {
          Index p = doc_sa.sa(j);
          Index lcp = doc_sa.lcp(j);
          text_lcp[start + p] = lcp;
        }
        num_docs++;
        start = i;
      }
    }
    std::vector<bool> visited(length);
    // permutate text_lcp[i] = text_lcp[sa[i]] implace
    for (Index i = 0; i < length; ++i) {
      if (!visited[i]) {
        int j = i;
        while (true) {
          visited[j] = 1;
          Index to = sa.sa(j);
          if (visited[to]) break;
          std::swap(text_lcp[j], text_lcp[to]);
          j = to;
        }
      }
      // ilcp[i] = text_lcp[sa.sa(i)];
    }
    wt_ = Wavelet(text_lcp.begin(), text_lcp.end());
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
