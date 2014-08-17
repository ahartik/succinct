#include "suffix-array.hpp"

#include <divsufsort.h>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

using std::string;
using Index = SuffixArray::Index;

SuffixArray::SuffixArray(const char* text, size_t len) {
  assert(len < std::numeric_limits<Index>::max());
  sa_.resize(len);
  lcp_.resize(len);
  text_.insert(text_.end(), text, text + len);
  
  const sauchar_t *sa_input = reinterpret_cast<const sauchar_t*>(text_.data());
  saint_t e = divsufsort(sa_input, &sa_[0], len);
  if (e != 0) {
    throw std::runtime_error("divsufsort failed");
  }
  // construct lcp
  // TODO: figure how to do this without sa_inv
  std::vector<Index> sa_inv(len);
  for (size_t i = 0; i < len; ++i) {
    sa_inv[sa_[i]] = i;
  }
  int l = 0;
  for (size_t i = 0; i < len; ++i) {
    Index k = sa_inv[i];
    if (k == 0) continue;
    Index j = sa_[k - 1];
    while (i + l < len && j + l < len && text[i + l] == text[j + l]) ++l;
    lcp_[k] = l;
    if (l > 0) --l;
  }
}

SuffixArray::SuffixArray(const SuffixArray& o) :
  text_(o.text_), sa_(o.sa_), lcp_(o.lcp_) { }

SuffixArray::SuffixArray(SuffixArray&& o) :
  text_(std::move(o.text_)), sa_(std::move(o.sa_)), lcp_(std::move(o.lcp_)) { }

int SuffixArray::compare(const string& pattern, Index p) const {
  int d = 0;
  for (Index i = 0; i < static_cast<Index>(pattern.size()) && d == 0; ++i) {
    if (p + i >= static_cast<Index>(text_.size())) {
      d = -1;
      break;
    }
    unsigned char c = text_[p + i];
    unsigned char pc = pattern[i];
    if (c < pc) d = -1;
    else if (c > pc) d = 1;
  }
  return d;
}

// returns first index p with suffix[p] <= pattern
Index SuffixArray::left_rec(const string& pattern, Index left, Index right) const {
  if (left + 1 >= right) {
    return right;
  }
  Index m = (left + right) >> 1;
  Index p = sa_[m];
  int c = compare(pattern, p);
  if (c < 0) { // suffix < pattern
    return left_rec(pattern, m, right);
  } else {
    assert(c == 0);
    return left_rec(pattern, left, m);
  }
}

// returns first index p with suffix[p] > pattern
Index SuffixArray::right_rec(const string& pattern, Index left, Index right) const {
  if (left + 1 >= right) {
    return right;
  }
  int m = (left + right) >> 1;
  int p = sa_[m];
  int c = compare(pattern, p);
  if (c > 0) { // suffix >= pattern
    return right_rec(pattern, left, m);
  } else {
    assert(c == 0);
    return right_rec(pattern, m, right);
  }
}

SuffixArray::SuffixRange SuffixArray::locate_rec(const string& pattern,
                                                Index left,
                                                Index right) const {
  if (left == right) {
    return SuffixRange(left, right);
  }
  int m = (left + right) >> 1;
  int p = sa_[m];
  int c = compare(pattern, p);
  if (c != 0 && left + 1 == right) return SuffixRange(left, left);
  if (c == 0) {
    return SuffixRange(left_rec(pattern, left, m),
                         right_rec(pattern, m, right));
  } else if (c < 0) {
    return locate_rec(pattern, m, right);
  } else {
    return locate_rec(pattern, left, m);
  }
}
SuffixArray::SuffixRange SuffixArray::locate(const string& pattern) const {
  SuffixRange ret = locate_rec(pattern, 0, sa_.size());
  return ret;
}
