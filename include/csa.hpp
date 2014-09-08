#pragma once

#include "delta-vector.hpp"
#include "fast-bit-vector.hpp"
#include "int-array.hpp"

#include <cassert>
#include <divsufsort.h>
#include <utility>
#include <algorithm>
#include <cstring>
#include <stdexcept>

template <typename DVector = DeltaVector<>>
class CSA {
 public:
  typedef int Index;
  typedef std::pair<Index,Index> SuffixRange;
  CSA(const char* input, size_t n) {
    const sauchar_t *sa_input = reinterpret_cast<const sauchar_t*>(input);
    std::vector<saidx_t> sa(n);
    saint_t e = divsufsort(sa_input, &sa[0], n);
    if (e != 0) {
      throw std::runtime_error("divsufsort failed");
    }

    memset(c_, 0, sizeof(c_));
    for (size_t i = 0; i < n; ++i)
      c_[sa_input[i]]++;
    for (int i = 1; i < 256; ++i) {
      c_[i] += c_[i-1];
    }

    size_ = n;
    // TODO: figure how to do this without sa_inv
    std::vector<saidx_t> sa_inv(n);
    for (size_t i = 0; i < n; ++i) {
      sa_inv[sa[i]] = i;
    }

    dvs_.reserve(256);
    // std::vector<int> succs;
    for (int c = 0; c < 256; ++c) {
      std::vector<saidx_t> succ_part;
      for (size_t i = c == 0 ? 0 : c_[c-1]; i != c_[c]; ++i) {
        if (size_t(sa[i]) == n-1) {
          last_pos_ = i;
          succ_part.push_back(0);
        } else {
          succ_part.push_back(1 + sa_inv[sa[i] + 1]);
        }
      }
      dvs_.emplace_back(succ_part);
      // succs.insert(succs.end(), succ_part.begin(), succ_part.end());
    }
//    std:: << "first_pos= " << sa_inv[0] << "\n";
    sa_inv = std::vector<saidx_t>();

    sample_w_ = 10;
    sample_mask_ =  (1ll << sample_w_) - 1;
    sample_.resize(1 + (n >> sample_w_));
    for (size_t i = 0; i < n; i+= (1 << sample_w_)) {
      int p = i >> sample_w_;
      sample_[p] = sa[i];
    }
//    std:: << "last_pos = " << last_pos_ << "\n";
#if 0
    for (size_t i = 0; i < n; ++i) {
      if (i != last_pos_) {
        assert(succs[i] - 1 == succ(i));
      }
    }
    for (size_t i = 0; i < n; ++i) {
//      // std:: << this->sa(i) << " == " << sa[i] << "\n";
      // assert(this->sa(i) == sa[i]);
    }
#endif
  }

  int succ(size_t i) const {
    assert(i != last_pos_);
    // if (i == last_pos_)
    int f = first(i);
    const DVector& dv = dvs_[f];
    int b = f == 0 ? 0 : c_[f - 1];
    return dv[i - b] - 1;
  }

  int sa(size_t i) const {
    int c = 0;
    while (i != last_pos_) {
//      // std:: << "i = " << i << "\n";
      if ((i & sample_mask_) == 0) {
        size_t s = sample_[(i >> sample_w_)];
        return s - c;
      }
      c++;
      i = succ(i);
    }
    return size_ - c -1;
  }

  unsigned char first(size_t i) const {
    return std::lower_bound(c_, c_+256, i+1) - c_;
  }

  SuffixRange locate(const std::string& pattern) const {
     Index begin = 0;
     Index end = size_;
     for (size_t i = 0; i < pattern.size(); ++i) {
//       std:: << "i = " << i << "\n";
//       std:: << "\tbegin = " << begin << "\n";
//       std:: << "\tend = " << end << "\n";
       unsigned char c = pattern[pattern.size() - i - 1];
       Index b = c == 0 ? 0 : c_[c - 1];
       begin = b + dvs_[c].lower_bound(begin + 1);
       end = b + dvs_[c].lower_bound(end + 1);
     }
//     std:: << "\tbegin = " << begin << "\n";
//     std:: << "\tend = " << end << "\n";
     return SuffixRange(begin, end);
  }

  size_t size() const {
    return size_;
  }
  size_t byteSize() const {
    size_t ret = sizeof(*this);
    ret += sample_.size() * sizeof(Index);
//    std:: << "sample size " << ret << "\n";
    size_t samples = 0;
    size_t deltas = 0;
    for (const auto& x : dvs_) {
      ret += x.byteSize();
      deltas += x.byteSize() - x.sampleSize();
      samples += x.sampleSize();
    }
//    std:: << "samples = " << samples << "\n";
//    std:: << "deltas = " << deltas << "\n";
    return ret;
  }
 private:
  std::vector<DVector> dvs_;
  size_t last_pos_;
  size_t c_[256];
  std::vector<Index> sample_;
  int sample_w_;
  size_t size_;
  uint64_t sample_mask_;
};
