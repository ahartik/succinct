#pragma once

#include <algorithm>

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <vector>

class SmallRMQ {
 public:
  SmallRMQ(size_t n) : n_(n), set_count_(0) {
  }
  template<typename Func>
  uint64_t addRMQ(Func func, size_t begin = 0) {
    uint64_t id = cartesianID<Func>(func, begin, begin + n_);
    if (stored.size() <= id)
      stored.resize(id + 1);
    std::vector<char> &table = stored[id];
    if (table.size() != 0) return id;
    set_count_++;
    table.resize(n_ * n_);
    for (int i = 0; i < n_; ++i) {
      char min = i;
      for (int j = i; j < n_; ++j) {
        if (func(begin + j) < func(begin + min)) {
          min = j;
        }
        table[i * n_ + j] = min;
      }
    }
    return id;
  }

  int rmq(uint64_t id, int begin, int end) const {
    assert(stored.size() > id);
    assert(stored[id].size() != 0);
    if (begin >= end) return begin;
    return stored[id][begin * n_ + (end - 1)];
  }
  size_t byteSize() const {
    return sizeof(*this) + stored.size() * sizeof(stored[0]) + n_ * n_ * set_count_;
  }
 private:
  template<typename Func>
  static uint64_t cartesianID(Func arr, size_t begin, size_t end) {
    size_t n = end - begin;
    if (n <= 1) return 0;
    assert(n < 32);
    size_t m = begin;
    for (size_t i = begin + 1; i < end; ++i) {
      if (arr(i) < arr(m)) m = i;
    }
    uint64_t ret = 0ull;
    if (m != begin) {
      // left child exists
      ret |= 1;
    }
    if (m != end - 1) {
      // right child exists
      ret |= 2;
    }
    uint64_t left_id = cartesianID<Func>(arr, begin, m);
    uint64_t right_id = cartesianID<Func>(arr, m + 1, end);
    ret |= left_id << 2;
    ret |= right_id << (2 + 2 * (m - begin));
    return ret;
  }

  size_t n_;
  size_t set_count_;
  std::vector<std::vector<char>> stored;
};

