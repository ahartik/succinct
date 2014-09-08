#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "bit-utils.hpp"

template<typename T>
class SparseIntArray {
 public:
  SparseIntArray(size_t n) : size_(n) {
    groups.resize(1 + n/64);
    zero = 0;
  }
  size_t size() const {
    return size_;
  }
  ~SparseIntArray() {
    for (Group& g : groups) {
      if (WordPopCount(g.exist) > 1) {
        delete[] g.u.values;
      }
    }
  }

  const T& operator[](size_t i) const {
    int o = i % 64;
    const Group& g = groups[i / 64];
    if ((g.exist & (1ull << o)) == 0) return zero;
    if (g.exist == (1ull << o)) return g.u.single;
    int r = WordPopCount(g.exist & ((1ull << o) - 1));
    return g.u.values[r];
  }
  const T& get(size_t i) const {
    return (*this)[i];
  }

  T& operator[](size_t i) {
    int o = i % 64;
    Group& g = groups[i / 64];
    if ((g.exist & (1ull << o)) == 0) {
      if (g.exist == 0) {
        g.exist = 1ull << o;
        g.u.single = 0;
        return g.u.single;
      }
      if (__builtin_popcountll(g.exist) == 1) {
        T old = g.u.single;
        g.u.values = new T[2];
        assert (g.exist != (1ull << o));
        if ((1ull << o) < g.exist) {
          g.exist |= 1ull << o;
          g.u.values[1] = old;
          g.u.values[0] = 0;
          return g.u.values[0];
        } else {
          g.exist |= 1ull << o;
          g.u.values[0] = old;
          g.u.values[1] = 0;
          return g.u.values[1];
        }
      }
      int k = __builtin_popcountll(g.exist);
      assert(k > 1);
      T* nv = new T[k + 1];
      g.exist |= 1ull << o;
      uint64_t v = g.exist;
      int r = -1;
      int p = 0;
      int op = 0;
      while (v) {
        int j = ffsll(v) - 1;
        v ^= 1ull << j;
        if (j == o) {
          r = p;
          nv[p] = 0;
          p++;
        } else {
          nv[p] = g.u.values[op];
          p++;
          op++;
        }
      }
      delete[] g.u.values;
      g.u.values = nv;
      return g.u.values[r];
    }
    if (g.exist == (1ull << o)) return g.u.single;
    int r = __builtin_popcountll(g.exist & ((1ull << o) - 1));
    return g.u.values[r];
  }
 private:
  struct Group {
    Group() : exist(0) {
      u.single = 0;
    }
    uint64_t exist;
    union {
      T *values;
      T single;
    } u;
  };
  std::vector<Group> groups;
  size_t size_;
  T zero;
};
