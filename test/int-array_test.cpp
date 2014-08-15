#include "int-array.h"
#include <vector>

#include <cassert>
#include <cstdlib>

#include <iostream>
using namespace std;

void test(IntArray& arr) {
  vector<uint64_t> ref(arr.size());
  for (int i = 0; i < arr.size(); ++i) {
    int j = rand() % arr.size();
    uint64_t v = rand() % arr.maxValue();
    arr.set(j, v);
    ref[j] = v;
    assert(ref[j] == arr.get(j));
  }
  for (int i = 0; i < arr.size(); ++i) {
    assert(ref[i] == arr.get(i));
  }
}

int main() {
  for (int w = 1; w < 64; ++w) {
    IntArray arr(w, 100000);
    test(arr);
    std::cout << "arr(" << w << ") OK! \n";
  }
}
