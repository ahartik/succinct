#pragma once

#include "balanced-wavelet.hpp"
#include "rle-wavelet.hpp"
#include "fast-bit-vector.hpp"
#include "int-array.hpp"

#include <divsufsort.h>
#include <vector>
#include <cassert>

// TODO: unhack!
template<typename Wavelet = BalancedWavelet<>>
class WtCSA {
 public:
  typedef int Index;
  typedef std::pair<Index,Index> SuffixRange;
  // typedef RLEWavelet<BalancedWavelet<>> Wavelet;
  // contents of input are bwtd afterwards
  WtCSA(const char* input, size_t n) {
    memset(c_, 0, sizeof(c_));
    // c_[0] = 1;
    for (size_t i = 0; i < n; ++i) {
      c_[(unsigned char)input[i]]++;
    }
    for (int i = 1; i < 256; ++i) {
      c_[i] += c_[i-1];
    }
    unsigned char last_char = input[n - 1];
    last_pos_ = last_char == 0 ? 0 : c_[last_char - 1];

    std::vector<unsigned char> input_copy(
        reinterpret_cast<const unsigned char*>(input),
        reinterpret_cast<const unsigned char*>(input + n));
    sauchar_t *bwt_input = &input_copy[0];
    saidx_t p;
    // bw_transform(bwt_input, bwt_input, nullptr, n, &p);
    p = divbwt(bwt_input, bwt_input, nullptr, n);
    // std::rotate(bwt_input, bwt_input + 1, bwt_input + p);
    // std::rotate(bwt_input, bwt_input + 1, bwt_input + last_pos_);
    
    wt_ = Wavelet(bwt_input, n);

    first_pos_ = p - 1;

    sample_w_ = 4;
    sample_mask_ =  (1ll << sample_w_) - 1;
    sample_.resize(1 + (n >> sample_w_));
    size_t r = first_pos_;
    size_t sa = 0;
    while (sa < n) {
      if ((r & sample_mask_) == 0ll) {
        size_t p = r >> sample_w_;
        assert(sample_[p] == 0);
        sample_[p] = sa;
      }
      sa++;
      if (sa < n)
        r = succ(r);
    }
  }

  size_t succ(size_t i) const {
    assert(i != last_pos_);
    int f = first(i);
    int cc = f == 0 ? 0 : c_[f - 1];
    int r = (int)i - cc;
//    //  std:: << "r = " << r << "\n";
//    //  std:: << "f = " << f << "\n";
//    //  std:: << "bwt = " << (char)wt_[i] << "\n";
    assert(r >= 0);
    size_t ret = wt_.select(r+1, f) - 1;
    if (ret <= first_pos_)
      --ret;
    return ret;
  }

  int sa(size_t i) const {
    int c = 0;
    while (i != last_pos_) {
      if ((i & sample_mask_) == 0) {
        size_t s = sample_[(i >> sample_w_)];
        return s - c;
      }
      c++;
      i = succ(i);
    }
    return wt_.size() - c - 1;
  }

  unsigned char first(size_t i) const {
    return std::lower_bound(c_, c_+256, i+1) - c_;
  }

  SuffixRange locate(const std::string& pattern) const {
     Index begin = 0;
     Index end = size();
     for (size_t i = 0; i < pattern.size(); ++i) {
//       std:: << "i = " << i << "\n";
//       std:: << "\tbegin = " << begin << "\n";
//       std:: << "\tend = " << end << "\n";
       unsigned char c = pattern[pattern.size() - i - 1];
//       std:: << "\tc = " << c << "\n";
       Index b = c == 0 ? 0 : c_[c - 1];
       begin = b + lower_bound(begin, c);
       end = b + lower_bound(end, c);
     }
//     std:: << "\tbegin = " << begin << "\n";
//     std:: << "\tend = " << end << "\n";
     return SuffixRange(begin, end);
  }
  size_t size() const {
    return wt_.size();
  }

  size_t byteSize() const {
    return sizeof(*this) + sample_.size() * sizeof(sample_[0]) +
        wt_.byteSize();
  }

 private:
  size_t lower_bound(size_t x, unsigned char c) const {
    if (x <= first_pos_) ++x;
    return wt_.rank(x, c);
  }

  Wavelet wt_;
  size_t last_pos_;
  size_t first_pos_;
  size_t c_[256];
  std::vector<size_t> sample_;
  int sample_w_;
  uint64_t sample_mask_;
};
