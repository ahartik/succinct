#pragma once
#include <iterator>
#include "bit-utils.hpp"

using std::ptrdiff_t;
class BitReference {
 public:
  BitReference() {
    offset = 0;
    data = nullptr;
  }
  BitReference(const BitReference& o) = default;
  BitReference operator=(const BitReference& o) = delete;
  bool operator=(bool b) {
    if (b) {
      *data |= (1ul << offset);
    } else {
      *data &= ~(1ul << offset);
    }
    return b;
  }
  bool operator==(const BitReference& o) const {
    return (bool)*this == (bool)o;
  }
  operator bool() const {
    return ((*data) >> offset) & 1;
  }

  int offset;
  unsigned long* data;
};

class ConstBitReference {
 public:
  ConstBitReference() {
    offset = 0;
    data = nullptr;
  }
  ConstBitReference(const ConstBitReference& o) = default;
  ConstBitReference operator=(const ConstBitReference& o) = delete;

  bool operator==(const ConstBitReference& o) const {
    return (bool)*this == (bool)o;
  }

  operator bool() const {
    return ((*data) >> offset) & 1;
  }

  int offset;
  const unsigned long* data;
};

template<typename Reference>
class BitIteratorImpl : public std::iterator<
                        std::random_access_iterator_tag,
                        bool,
                        ptrdiff_t,
                        bool*,
                        Reference>
{
 public:
  typedef BitIteratorImpl<Reference> BitIterator;
  BitIteratorImpl() {
    ref.offset = 0;
    ref.data = nullptr;
  }
  BitIteratorImpl(Reference r) : ref(r) {}
  BitIteratorImpl(const BitIteratorImpl& o) = default;

  bool operator==(const BitIterator& o) const {
    return
        ref.offset == o.ref.offset &&
        ref.data == o.ref.data;
  }
  bool operator!=(const BitIterator& o) const {
    return !(*this == o);
  }
  bool operator<(const BitIterator& o) const {
    if (ref.data == o.ref.data)
      return ref.offset < o.ref.offset;
    return ref.data < o.ref.data;
  }
  bool operator>(const BitIterator& o) const {
    return o < (*this);
  }
  bool operator<=(const BitIterator& o) const {
    return (*this) < o || (*this) == o;
  }
  bool operator>=(const BitIterator& o) const {
    return o <= (*this);
  }
  bool operator*() const {
    return (bool)ref;
  }
  Reference operator*() {
    return ref;
  }
  BitIterator operator--() {
    if (ref.offset == 0) {
      ref.data--;
      ref.offset = WordBits;
    }
    ref.offset--;
    return *this;
  }
  BitIterator operator++() {
    if (ref.offset == WordBits-1) {
      ref.data++;
      ref.offset = -1;
    }
    ref.offset++;
    return *this;
  }
  BitIterator operator--(int x) {
    BitIterator result = *this;
    --(*this);
    return result;
  }
  BitIterator operator++(int x) {
    BitIterator result = *this;
    ++(*this);
    return result;
  }
  
  BitIterator operator-=(ptrdiff_t d) {
    return (*this) += -d;
  }
  ptrdiff_t operator-(BitIterator o) const {
    ptrdiff_t r = ref.data - o.ref.data;
    r *= 64;
    r += ref.offset - o.ref.offset;
    return r;
  }

  BitIterator operator+=(ptrdiff_t d) {
    bool sign = d >= 0;
    d = llabs(d);
    ptrdiff_t ptr_move = d / WordBits;
    ptrdiff_t offset_move = d % WordBits;
    if (sign) {
      // positive
      ref.data += ptr_move;
      ref.offset += offset_move;
      if (ref.offset > WordBits) {
        ref.data++;
        ref.offset -= WordBits;
      }
    } else {
      // negative
      ref.data -= ptr_move;
      ref.offset -= offset_move;
      if (ref.offset < 0) { 
        ref.offset += WordBits;
        ref.data --;
      }
    }
    assert(ref.offset >= 0);
    assert(ref.offset < WordBits);
    return *this;
  }
  friend BitIterator operator+(ptrdiff_t d, BitIterator it) {
    return it + d;
  }
  
  friend BitIterator operator+(BitIterator it, ptrdiff_t d) {
    it += d;
    return it;
  }
  friend BitIterator operator-(BitIterator a, ptrdiff_t d) {
    return a + (-d);
  }
 private:
  Reference ref;
};

typedef BitIteratorImpl<BitReference> BitIterator;
typedef BitIteratorImpl<ConstBitReference> ConstBitIterator;
