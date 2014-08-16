#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

static const int WordBits = 64;
typedef uint64_t Word;

// Select for single word.
//  v: Input value to find position with rank r.
//  r: bit's desired rank [1-64].
// Returns: First index i so that r == rank(v,i)
int InitBitUtils();
extern uint8_t BytePop[256];
extern uint8_t ByteSelect[256][8];
extern Word BinTable[65][64];

// Select for single word.
//  v: Input value to find position with rank r.
//  r: bit's desired rank [1-64].
// Returns: First index i so that r == rank(v,i)
static inline int WordSelect(Word v, int r) {
  static int init = InitBitUtils();
  (void) init;
  for (size_t b = 0; b < sizeof(long); ++b) {
    int c = BytePop[v&255];
    if (c >= r) {
      return b * 8 + ByteSelect[v&255][r-1];
    }
    r -= c;
    v >>= 8;
  }
  assert(false);
  return -1;
}
static inline int WordPopCount(Word p) {
  return __builtin_popcountll(p);
}

static inline int WordLog2(Word x) {
  return 63 - __builtin_clzll(x);
}

static inline Word Binomial(int k, int n) {
  static int init = InitBitUtils();
  (void) init;
  if (k == n) return 1;
  if (k > n) return 0;
  assert (n <= 64);
  return BinTable[n][k];
}
