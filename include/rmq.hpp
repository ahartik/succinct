#pragma once

#include "int-array.hpp"
#include "bit-utils.hpp"

#include "small-rmq.hpp"

#include <cmath>
#include <chrono>
#include <vector>
#include <cstddef>
#include <memory>
#include <functional>
#include <algorithm>

#include <iostream>

template<typename T>
struct ArrWrap {
  ArrWrap(const T* arr) {
    arr_ = arr;
  }
  const T& operator()(size_t i) const {
    return arr_[i];
  }
  const T* arr_;
};

// Solution in <O(n log n), O(1)>
template<typename T, typename Func = ArrWrap<T>>
class FastRMQ {
 public:
  FastRMQ(Func arr, size_t n) : arr_(arr) {
    m.resize(1 + WordLog2(n-1));
    size_t size = 0;
    for (size_t w = 0; w < m.size(); ++w) {
      m[w] = IntArray(w + 1, n);
      size += m[w].byteSize();
      m[w].set(0,0);
    }
    for (size_t i = 1; i < n; ++i) {
      if (arr(i) < arr(i-1)) {
        m[0].set(i, 0);
      } else {
        m[0].set(i, 1);
      }
    }
    for (size_t w = 1; w < m.size(); ++w) {
      int span = 1 << w;
      for (size_t i = 0; i < n; ++i) {
        int a = i - m[w-1].get(i);
        int bi = std::max<int>(i - span, 0);
        int b = bi - m[w-1].get(bi);
        if (arr(a) < arr(b)) {
          m[w].set(i, i - a);
        } else {
          m[w].set(i, i - b);
        }
      }
    }
  }
  size_t rmq(size_t begin, size_t end) const {
    if (begin + 1 >= end)return begin;
    if (begin + 2 == end) {
      return end - 1 - m[0].get(end - 1);
    }
    int w = WordLog2(end - begin - 1);
    assert(begin + (1<<w) < end);
    assert(begin + (2<<w) >= end);

    int ai = end - 1;
    int a = ai - m[w-1].get(ai);
    int bi = begin + (1 << w) - 1;
    int b = bi - m[w-1].get(bi);

    if (arr_(a) < arr_(b)) return a;
    else return b;
  }
 private:
  Func arr_;
  std::vector<IntArray> m;
};

template<typename T, typename Func = ArrWrap<T>>
class RMQ {
 private:
  typedef std::function<const T&(size_t i)> FuncWrap;
 public:
  RMQ(Func arr, size_t n, int depth = 2) 
      : size_(n),
        arr_(arr),
        small_rmq_(1)
  {
    bs_ = (1 + WordLog2(n-1)) / 3;
    if (n <= 1 || bs_ <= 1) {
      bs_ = 0;
    } else {
      size_t num_blocks = 1 + (n-1) / bs_;
      small_rmq_ = SmallRMQ(bs_);

      block_min_ = IntArray(1 + WordLog2(bs_ - 1), num_blocks);
      block_id_ = IntArray(2 * bs_, num_blocks);
      for (size_t i = 0; i < num_blocks; ++i) {
        size_t m = 0;
        size_t start = i * bs_;
        for (size_t j = 1; j < bs_; ++j) {
          if (arr_(start + j) < arr_(start + m)) {
            m = j;
          }
        }
        block_min_.set(i, m);
        uint64_t block_id = small_rmq_.addRMQ<Func>(arr, start);
        block_id_.set(i, block_id);
      } 
      using namespace std::placeholders;
      BlockFunc block_func {this};
      if (depth == 0) {
        block_fast_.reset(new FastRMQ<T, BlockFunc>(block_func, num_blocks));
      } else {
        FuncWrap wrapped = block_func;
        block_rec_.reset(new RMQ<T, FuncWrap>(wrapped, num_blocks, depth-1));
      }
    }
  }
  size_t rmq(size_t begin, size_t end) const {
    if (bs_ == 0) return brute_rmq(begin, end);
    if (begin == end) return end;
    size_t start_block = 1 + (begin / bs_);
    size_t end_block = end / bs_;

    size_t ret = begin;
    if (start_block > end_block) {
      return brute_rmq(begin, end);
    }
    if (start_block < end_block) {
      size_t min_block = blockRMQ(start_block, end_block);
      size_t middle_min = min_block * bs_ + block_min_.get(min_block);
      ret = middle_min;
    }
    assert(begin <= start_block * bs_);
    assert(end >= end_block * bs_);

    if (begin != start_block * bs_) {
      size_t start_min = brute_rmq(begin, start_block * bs_);
      if (arr_(start_min) <= arr_(ret))
        ret = start_min;
    }
    if (end != end_block * bs_) {
      size_t end_min = brute_rmq(end_block * bs_, end);
      if (arr_(end_min) < arr_(ret))
        ret = end_min;
    }
    assert(begin <= ret && ret < end);
    return ret;
  }
 private:
  size_t brute_rmq(size_t begin, size_t end) const {
    if (begin == end) return end;
    if (bs_ != 0) {
      size_t block = begin / bs_;
      begin -= block * bs_;
      end -= block * bs_;
      assert(begin >= 0 && end <= bs_);
      return block * bs_ + small_rmq_.rmq(block_id_.get(block), begin, end);
    }
    size_t r = begin;
    for (size_t i = begin + 1; i < end; ++i) {
      if (arr_(i) < arr_(r)) r = i;
    }
    return r;
  }

  const T& blockMin(size_t i) const {
    size_t block_start = bs_ * i;
    return arr_(block_start + block_min_.get(i));
  }

  struct BlockFunc {
    const RMQ<T, Func>* rmq;
    const T& operator()(size_t i) const {
      return rmq->blockMin(i);
    }
  };
  size_t blockRMQ(size_t begin, size_t end) const {
    if (block_rec_ != nullptr) {
      return block_rec_->rmq(begin, end);
    } else {
      return block_fast_->rmq(begin, end);
    }
  }

  size_t bs_;
  size_t size_;

  IntArray block_min_;
  Func arr_;
  std::unique_ptr<RMQ<T, FuncWrap>> block_rec_;
  std::unique_ptr<FastRMQ<T, BlockFunc>> block_fast_;

  SmallRMQ small_rmq_;
  IntArray block_id_;

};
