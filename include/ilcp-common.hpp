#pragma once

#include "suffix-array.hpp"

void ILCPConstruct(const SuffixArray& sa,
                   std::vector<SuffixArray::Index>* ilcp) {
  typedef SuffixArray::Index Index;
  std::vector<Index>& text_lcp = *ilcp;
  text_lcp.resize(sa.size());
  Index start = 0;
  int num_docs = 0;
  const char* text = sa.text();
  for (Index i = 0; i <= sa.size(); ++i) {
    if (i == sa.size() || (unsigned char)text[i] <= 1) {
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
  std::vector<bool> visited(sa.size());
  // permutate text_lcp[i] = text_lcp[sa[i]] implace
  for (Index i = 0; i < sa.size(); ++i) {
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
}
