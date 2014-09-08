#pragma once
// Simple suffix array
#include <vector>
#include <utility>
#include <cstddef>
#include <string>

class SuffixArray {
 public:
  typedef int Index;
  typedef std::pair<Index, Index> SuffixRange;
  SuffixArray(const char* text, size_t length);
  SuffixArray(SuffixArray&& o);
  SuffixArray(const SuffixArray& o);
  SuffixRange locate(const std::string& pattern) const;

  Index lcp(Index i) const {
    return lcp_[i];
  }

  Index sa(Index i) const {
    return sa_[i];
  }

  const char* text() const {
    return text_.data();
  }

  size_t size() const {
    return text_.size();
  }
  
  const Index* sa_data() const {
    return &sa_[0];
  }

  const Index* lcp_data() const {
    return &lcp_[0];
  }
 private:
  int compare(const std::string& pattern, Index p) const;
  Index left_rec(const std::string& pattern, Index left, Index right) const;
  Index right_rec(const std::string& pattern, Index left, Index right) const;
  SuffixRange locate_rec(const std::string& pattern, Index left, Index right) const;
  std::vector<char> text_;
  std::vector<Index> sa_;
  std::vector<Index> lcp_;
};
