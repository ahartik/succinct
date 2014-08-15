#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

static const int WordBits = 8 * sizeof(long);

// Select for single word.
//  v: Input value to find position with rank r.
//  r: bit's desired rank [1-64].
// Returns: First index i so that r == rank(v,i)
int BUInitTables();
extern uint8_t BytePop[256];
extern uint8_t ByteSelect[256][8];


// Select for single word.
//  v: Input value to find position with rank r.
//  r: bit's desired rank [1-64].
// Returns: First index i so that r == rank(v,i)
static inline int WordSelect(unsigned long v, int r) {
  static int init = BUInitTables();
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
