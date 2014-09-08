#include "suffix-array.hpp"
#include "ilcp-common.hpp"
#include "rle-sparse-vector.hpp"
#include "rle-delta-vector.hpp"

class RLEILCPCount {
 public:
  typedef SuffixArray::Index Index;
  typedef SuffixArray::SuffixRange SuffixRange;
  typedef RLEDeltaVector<> Vector;
  RLEILCPCount(const SuffixArray& sa) {
    Index length = sa.size();
    std::vector<Index> ilcp(length);
    ILCPConstruct(sa, &ilcp);

    std::vector<std::vector<Index>> pos;
    for (Index i = 0; i < (Index)sa.size(); ++i) {
      Index val = ilcp[i];
      if (Index(pos.size()) <= val) {
        pos.resize(val + 1);
      }
      pos[ilcp[i]].push_back(i);
    }

    for (size_t v = 0; v < pos.size(); ++v) {
      if (!pos[v].empty()) {
        val_.push_back(v);
        pos_.emplace_back(pos[v]);
        // Release memory:
        pos[v] = std::vector<Index>();
      }
    }
  }

  size_t count(const SuffixRange& range, const std::string& pattern) const {
    size_t ret = 0;
    for (size_t i = 0; i < pos_.size(); ++i) {
      Index val = val_[i];
      if (size_t(val) >= pattern.size()) break;
      ret += pos_[i].lower_bound(range.second) -
             pos_[i].lower_bound(range.first);
    }
    return ret;
  }

  size_t count(const SuffixArray& sa, const std::string& pattern) const {
    SuffixArray::SuffixRange range = sa.locate(pattern);
    return count(range, pattern);
  }

  size_t byteSize() const {
    size_t ret = 0;
    ret += val_.size() * sizeof(Index);
    for (size_t i = 0; i < pos_.size(); ++i) {
      ret += pos_[i].byteSize();
#if 0
      std::cout << "pos(" << val_[i] << ") - " << pos_[i].byteSize()
                << " / " << pos_[i].size() << "\n";
#endif
    }
    ret += sizeof(*this);
    return ret;
  }

 private:
  std::vector<Index> val_;
  std::vector<Vector> pos_;
};
