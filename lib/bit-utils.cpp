#include "bit-utils.hpp"
#include <mutex>
#include <cstring>

uint8_t BytePop[256];
uint8_t ByteSelect[256][8];
Word BinTable[64][64];

static std::mutex table_mutex;

int InitBitUtils() {
  std::lock_guard<std::mutex> lock(table_mutex);
  static bool initialized = false;
  if (initialized) return 1;
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

  memset(BinTable, 0, sizeof(BinTable));
  BinTable[0][0] = 1;
  BinTable[1][1] = 1;
  BinTable[1][0] = 1;
  for (int i = 2; i < 64; ++i) {
    BinTable[i][0] = 1;
    for (int j = 1; j < i; ++j) {
      BinTable[i][j] = BinTable[i - 1][j - 1] + BinTable[i - 1][j];
    }
    BinTable[i][i] = 1;
  }
  initialized = true;
  return 1;
}
