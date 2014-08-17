#ifndef SKEWED_WAVELET_H
#define SKEWED_WAVELET_H

#include "fast-bit-vector.hpp"
#include "balanced-wavelet.hpp"
#include <stdint.h>
#include <vector>

template<typename BitVector = FastBitVector>
class SkewedWavelet {
  static const int StartSize = 2;
  static const int StartBits = 1;
  static const int MaxLevel = 60;
 public:
  // Empty constructor
  SkewedWavelet() {
  }

  template<typename IntType>
  SkewedWavelet(IntType* arr, size_t size) {
    std::vector<size_t> level_size(MaxLevel);
    std::vector<MutableBitVector> pick(MaxLevel);
    for (size_t i = 0; i < size; ++i) {
      int64_t fixed;
      int lvl = Level(arr[i], &fixed);
      level_size[lvl]++;
      for (int j = 0; j < lvl; ++j)
        pick[j].push_back(0);
      pick[lvl].push_back(1);
    }
    wt_pick_.reserve(MaxLevel);
    for (int lvl = 0; lvl < MaxLevel; ++lvl) {
      wt_pick_.emplace_back(pick[lvl]);
    }
    wt_.reserve(MaxLevel);

    // reorder array by levels
    std::stable_sort(arr, arr + size, &LevelCmp);
    size_t start = 0;;
    for (size_t j = 0; j < MaxLevel; ++j) {
      for (size_t i = 0; i < level_size[j]; ++i) {
        int64_t fixed;
        int64_t a = arr[start + i];
        int lvl = Level(a, &fixed);
        (void)lvl;
        assert(lvl == j);
        arr[start + i] = fixed;
      }
      wt_.emplace_back(arr + start, arr + start + level_size[j]);
      start += level_size[j];
    }
  }
  template<typename It>
  SkewedWavelet(It begin, It end) {
    std::vector<BalancedWaveletEncoder> lv;
    for (int i = 0; i < MaxLevel; ++i)
      lv.emplace_back(i+1);

    std::vector<MutableBitVector> pick(MaxLevel);
    for (It it = begin; it != end; ++it) {
      int64_t fixed;
      int lvl = Level(*it, &fixed);
      lv[lvl].append(fixed);
      for (int j = 0; j < lvl; ++j)
        pick[j].push_back(0);
      pick[lvl].push_back(1);
    }
    wt_.reserve(MaxLevel);
    wt_pick_.reserve(MaxLevel);
    for (int lvl = 0; lvl < MaxLevel; ++lvl) {
      wt_pick_.emplace_back(pick[lvl]);
    }
    for (int lvl = 0; lvl < MaxLevel; ++lvl) {
      wt_.emplace_back(std::move(lv[lvl]));
    }
  }

  SkewedWavelet(SkewedWavelet&& o)
    : wt_(std::move(o.wt_)),
      wt_pick_(std::move(o.wt_pick_)) {
  }
  const SkewedWavelet& operator=(SkewedWavelet&& o) {
    wt_ = std::move(o.wt_);
    wt_pick_ = std::move(o.wt_pick_);
    return *this;
  }

  size_t rank(size_t pos, int64_t value) const {
    int64_t fixed = 0;
    int lvl = Level(value, &fixed);
    for (int i = 0; i < lvl; ++i) {
      pos = wt_pick_[i].rank(pos, 0);
    }
    pos = wt_pick_[lvl].rank(pos, 1);
    return wt_[lvl].rank(pos, fixed);
  }
  size_t rankLE(size_t pos, int64_t value) const {
    int64_t fixed = 0;
    int lvl = Level(value, &fixed);
    int64_t ret = 0;
    for (int i = 0; i < lvl; ++i) {
      size_t np = wt_pick_[i].rank(pos, 0);
      ret += pos - np;
      pos = np;
    }
    pos = wt_pick_[lvl].rank(pos, 1);
    return ret + wt_[lvl].rankLE(pos, fixed);
  }
  size_t size() const {
    return wt_pick_[0].size();
  }
  size_t byteSize() const {
    size_t ret = sizeof(*this);
    for (size_t i = 0; i < wt_pick_.size(); ++i) {
      ret += wt_pick_[i].byteSize();
      ret += wt_[i].byteSize();
    }
    return ret;
  }
  class Iterator {
   public:
    typedef typename BalancedWavelet<BitVector>::Iterator BalancedIterator;
    Iterator(const SkewedWavelet& sk) : wt(&sk) {
      spine = true;
      level = 0;
      level_start = 0;
    }
    const Iterator& operator=(const Iterator& o) {
      spine = o.spine;
      level = o.level;
      level_start = o.level_start;
      wt = o.wt;
      balanced_it = o.balanced_it;
      return *this;
    }
    bool isLeaf() const {
      if (spine) {
        return level == MaxLevel - 1;
      }
      return balanced_it.isLeaf();
    }
    uint64_t splitValue() const {
      if (spine) {
        return level_start + (StartSize << level);
      }
      return level_start + balanced_it.splitValue(); 
    }
    bool operator[](size_t i) const {
      if (spine) {
        return !wt->wt_pick_[level][i];
      } else {
        return balanced_it[i];
      }
    }
    size_t rank(size_t pos, bool bit) const {
      if (spine) {
        return wt->wt_pick_[level].rank(pos, !bit);
      } else {
        return balanced_it.rank(pos, bit);
      }
    }
    size_t select(size_t idx, bool bit) const {
      if (spine) {
        return wt->wt_pick_[level].select(idx, !bit);
      } else {
        return balanced_it.select(idx, bit);
      }
    }
    size_t count() const {
      if (spine) {
        return wt->wt_pick_[level].size();
      } else {
        return balanced_it.count();
      }
    }
    Iterator child(bool right) const {
      if (spine) {
        Iterator ret(*wt);
        if (right) {
          ret.spine = true;
          ret.level = level + 1;
          ret.level_start = level_start + (StartSize << level);
        } else {
          ret.spine = false;
          ret.level = level;
          ret.level_start = level_start;
          ret.balanced_it = BalancedIterator(wt->wt_[level]);
        }
        return ret;
      } else {
        Iterator ret(*wt);
        ret.spine = false;
        ret.level = level;
        ret.level_start = level_start;
        ret.balanced_it = balanced_it.child(right);
        return ret;
      }
    }
   private:
    bool spine;
    int level;
    uint64_t level_start;
    const SkewedWavelet* wt;
    BalancedIterator balanced_it;
  };
 private:
  static int Level(int64_t x, int64_t* fix) {
    int64_t size = StartSize;
    int64_t start = 0;
    int64_t end = StartSize;
    for (int i = 0; i < MaxLevel; ++i) {
      if (x < end) {
        *fix = x - start;
        return i;
      }
      size *= 2;
      start = end;
      end = start + size;
    }
    assert(false);
    *fix = 0;
    return 0;
  }

  static bool LevelCmp(int64_t a, int64_t b) {
    int64_t fix;
    return Level(a, &fix) < Level(b, &fix);
  }

  std::vector<BalancedWavelet<BitVector>> wt_;
  std::vector<BitVector> wt_pick_;
};

#endif
