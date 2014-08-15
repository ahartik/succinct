#include "bit-utils.hpp"

uint8_t BytePop[256];
uint8_t ByteSelect[256][8];

int BUInitTables() {
  for (int x = 0; x < 256; ++x) {
    BytePop[x] = __builtin_popcountll(x);
  }
  for (int x = 0; x < 256; ++x) {
    int r = 0;
    for (int i = 0; i < 8; ++i) {
      if ((x>>i)&1) {
        ByteSelect[x][r] = i+1;
        r++;
      }
    }
  }
  return 0;
}
