#ifndef RLE_WAVELET_H
#define RLE_WAVELET_H

#include <algorithm>
#include <iostream>

#include "fast-bit-vector.hpp"
#include "sparse-bit-vector.hpp"
#include "skewed-wavelet.hpp"

template<typename Wavelet = BalancedWavelet<>>
class RLEWavelet {
 public:
  template<typename It>
  RLEWavelet(It begin, It end) {
    if (begin == end) return;
    typedef typename std::remove_reference<decltype(*begin)>::type ValueInt;
    size_t run_pos = 0;
    std::vector<ValueInt> head;
    std::vector<size_t> run_end;
    head.push_back(*begin);
    size_t i = 0;
    std::vector<std::vector<size_t>> run;
    for (It it = begin; it != end; ++it, ++i) {
      if (*it != head.back()) {
        ValueInt last = head.back();
        run_end.push_back(i);
        if (run.size() <= size_t(last)) {
          run.resize(last + 1);
        }
        run[last].push_back(run_pos);
        head.push_back(*it);
        run_pos = 0;
      }
      run_pos++;
    }
    {
      ValueInt last = head.back();
      run_end.push_back(i);
      if (run.size() <= size_t(last)) {
        run.resize(last + 1);
      }
      run[last].push_back(run_pos);
      run_pos = 0;
    }
    assert(head.size() == run_end.size());
    run_end_ = SparseBitVector(run_end.begin(), run_end.end());
    run_end.clear();
    std::vector<size_t> num_rank;
    num_rank.resize(run.size());
    std::vector<size_t>& run_lens = run_end;
    size_t total = 0;
    for (size_t i = 0; i < run.size(); ++i) {
      num_rank[i] = run_lens.size();
      for (size_t j = 0; j < run[i].size(); ++j) {
        total += run[i][j];
        run_lens.push_back(total-1);
      }
      run[i] = std::vector<size_t>();
    }
    assert(run_lens.size() == head.size());
    head_ = Wavelet(&head[0], head.size()); // .begin(), head.end());
    run_len_ = SparseBitVector(run_lens.begin(), run_lens.end());
    num_rank_ = SparseBitVector(num_rank.begin(), num_rank.end());
    assert(run_lens.size() == run_len_.count(1));
  }
  RLEWavelet() { }
  RLEWavelet(RLEWavelet&& o) :
    head_(std::move(o.head_)),
    run_len_(std::move(o.run_len_)),
    num_rank_(std::move(o.num_rank_)),
    run_end_(std::move(o.run_end))
  { }
  const RLEWavelet& operator=(RLEWavelet&& o) {
    head_ = std::move(o.head_);
    run_len_ = std::move(o.run_len_);
    num_rank_ = std::move(o.num_rank_);
    run_end_ = std::move(o.run_end_);
    return *this;
  }

  size_t rank(size_t pos, uint64_t value) const {
    if (pos == 0) return 0;
    size_t rpos = headPos(pos);
    typename Wavelet::Iterator it(head_);
    bool eq = true;
    size_t hrank = rpos;
    for (;;) {
      bool bit = value >= it.splitValue();
      if (it[hrank] != bit) eq = false;
      hrank = it.rank(hrank, bit);
      if (it.isLeaf()) {
        break;
      }
      it = it.child(bit);
    }
    size_t begin = runRank(value, hrank);
    size_t run_start = 0;
    if (!eq) {
      return begin;
    }
    if (rpos != 0) run_start = run_end_.select1(rpos) - 1;
    size_t end = pos - run_start;
    return begin + end;
  }

  size_t rankLE(size_t pos, uint64_t value) const {
    typename Wavelet::Iterator it(head_);
    if (pos == 0) return 0;
    size_t rpos = headPos(pos);
    bool lt = true;
    size_t begin = rankLE(it, rpos, value, &lt);
    size_t run_start = 0;
    if (!lt) {
      return begin;
    }
    if (rpos != 0) run_start = run_end_.select1(rpos) - 1;
    size_t end = pos - run_start;
    return begin + end;
  }

  // TODO
  // size_t select(size_t rank, uint64_t value) const {
  //   size_t run = 
  // }

  size_t size() const {
    return run_end_.size();
  }

  size_t bitSize() const {
    size_t total = head_.bitSize();
    total += run_end_.bitSize();
    total += run_len_.bitSize();
    total += num_rank_.bitSize();
    return total;
  }

  uint64_t operator[](size_t i) const {
    return head_[headPos(i)];
  }

 private:
  size_t headPos(size_t pos) const {
    return run_end_.rank(pos + 1, 1);
  }

  // Internal recursive function for rankLE traversal.
  // *lt is set to false if head_[pos] > value.
  size_t rankLE(typename Wavelet::Iterator it, size_t pos, uint64_t value, bool* lt) const {
    if (it.count() == 0) return 0;
    size_t pos1 = it.rank(pos, 1);
    size_t pos0 = pos - pos1;
    uint64_t split = it.splitValue();
    bool b = value >= split;
    bool itb = it[pos];
    // All values in the future will be smaller than value
    if (itb < b) lt = nullptr;
    if (lt != nullptr && b < itb) *lt = false;
    if (it.isLeaf()) {
      size_t ret = 0;
      if (b) {
        ret += runRank(split, pos1);
      }
      if (pos0 != 0) {
        ret += runRank(split - 1, pos0);
      }
      return ret;
    }
    if (b) {
      size_t ret = 0;
      // Only carry lt to larger child.
      ret += rankLE(it.child(0), pos0, value, nullptr);
      ret += rankLE(it.child(1), pos1, value, lt);
      return ret;
    } else {
      return rankLE(it.child(0), pos0, value, lt);
    }
  }

  size_t runRank(uint64_t x, size_t runs) const {
    if (runs == 0) return 0;
    size_t num_rank = num_rank_.select1(x+1) - 1 ;
    size_t ret = run_len_.select1(num_rank + runs) -
                 run_len_.select1(num_rank);
    return ret;
  }

  SparseBitVector run_end_;
  SparseBitVector run_len_;
  SparseBitVector num_rank_;
  Wavelet head_;
};

#endif
