#pragma once
#include "sparse-int-array.hpp"
#include "suffix-array.hpp"
#include "sparse-bit-vector.hpp"

#include <vector>

void SadaConstruct (const SuffixArray& sa, SparseIntArray<int>* count) {
  assert((size_t)sa.size() == count->size());

  typedef SuffixArray::Index Index;
  using std::vector;
  vector<Index> ends;
  const unsigned char* text =
      reinterpret_cast<const unsigned char*>(sa.text());
  for (Index i = 0; i < (Index)sa.size(); ++i) {
    if (text[i] <= 1) {
      ends.push_back(i);
    }
  }
  SparseBitVector da(ends.begin(), ends.end());

  vector<int> prev(ends.size() + 1, -1);
  SparseIntArray<int>& counts = *count;

  // Keep track of the suffix tree path from root:
  vector<int> node_lcp;
  vector<int> min_pos;
  vector<int> max_pos;
  for (size_t i = 0; i < sa.size(); ++i) {
    assert(node_lcp.size() == min_pos.size());
    assert(node_lcp.size() == max_pos.size());
    int d = da.rank(sa.sa(i), 1);
    int lcp = sa.lcp(i);
    while (node_lcp.size() > 0 && node_lcp.back() > lcp) {
      min_pos.pop_back();
      max_pos.pop_back();
      node_lcp.pop_back();
    }
    if (node_lcp.size() > 0 && node_lcp.back() == lcp) {
      max_pos.back() = i;
    } else {
      node_lcp.push_back(lcp);
      min_pos.push_back(i);
      max_pos.push_back(i);
    }

    if (prev[d] == -1) {
      prev[d] = i;
    } else {
      int r = std::lower_bound(max_pos.begin(),
                               max_pos.end(),
                               prev[d] + 1) - max_pos.begin();
      r = min_pos[r];
      counts[r]++;
      prev[d] = i;
    }
  }
}
