#pragma once
#include "sparse-int-array.hpp"
#include "suffix-array.hpp"
#include "rmq.hpp"
#include "sparse-bit-vector.hpp"

#include <vector>

#define SADA_ONE_NODE 1

#if !SADA_ONE_NODE
void SadaConstruct (const SuffixArray& sa,
                    SparseIntArray<int>* count) {
  assert(sa.size() == count->size());

  typedef SuffixArray::Index Index;
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
  SparseIntArray<int>& counts = *count;
  for (int i = 0; i < sa.size(); ++i) {
    int d = da.rank(sa.sa(i), 1);
    if (prev[d] == -1) {
      prev[d] = i;
    } else {
      int r = lcp_rmq.rmq(prev[d]+1, i+1);
      int i2 = std::upper_bound(max_pos.begin() + prev[d],
      counts[r]++;
      prev[d] = i;
    }
  }
}

#else 

void SadaConstruct (const SuffixArray& sa,
                    SparseIntArray<int>* count) {
  assert(sa.size() == count->size());

  typedef SuffixArray::Index Index;
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

  vector<int> prev(ends.size() + 1, -1);
  SparseIntArray<int>& counts = *count;

  vector<int> stack;
  vector<int> min_pos;
  vector<int> max_pos;
  for (int i = 0; i < sa.size(); ++i) {
    int d = da.rank(sa.sa(i), 1);
    int lcp = sa.lcp(i);
    while (stack.size() > 0 && stack.back() > lcp) {
      min_pos.pop_back();
      max_pos.pop_back();
      stack.pop_back();
    }
    if (stack.size() > 0 && stack.back() == lcp) {
      max_pos.back() = i;
    } else {
      stack.push_back(lcp);
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
      // r = max_pos[r];
      counts[r]++;
      prev[d] = i;
    }
  }
#endif
}
