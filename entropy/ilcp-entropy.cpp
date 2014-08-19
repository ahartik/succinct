#include <iostream>
#include <fstream>
#include <gflags/gflags.h>
#include "suffix-array.hpp"
#include "ilcp-common.hpp"
#include <cmath>

bool readBinaryFile(const std::string& filename, std::vector<char> *contents) {
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (in)
  {
    in.seekg(0, std::ios::end);
    contents->resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&(*contents)[0], contents->size());
    in.close();
    return true;
  }
  return false;
}

double entropy(const std::vector<int>& counts) {
  double total = 0;
  for (int x : counts) {
    total += x;
  }
  double e = 0;
  for (int x : counts) {
    if (x == 0) continue;
    e += -x * log2(x / total);
  }
  return e;
}

DEFINE_string(file, "", "Input document collection.");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("usage: ./raw-ilcp --file doc_collection");
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  if (FLAGS_file.empty()) {
    std::cout << gflags::ProgramUsage() << "\n";
    return 1;
  }
  std::vector<char> contents;
  if (!readBinaryFile(FLAGS_file, &contents)) {
    std::cout << "Failed to read " << FLAGS_file << "\n";
    return 1;
  }
  std::vector<SuffixArray::Index> ilcp;
  SuffixArray sa(contents.data(), contents.size());
  ILCPConstruct(sa, &ilcp);
  std::vector<int> len_count;
  std::vector<int> val_count[2];
  int len = 1;
  int head = -1;
  for (size_t i = 0; i < ilcp.size(); ++i) {
    int x = ilcp[i];
    int d = x - head;
    if (d != 0) {
      if (i != 0) {
        if (len_count.size() <= len) {
          len_count.resize(len + 1);
        }
        len_count[len]++;
        len = 0;
      }
      d = x;
      int s = 0;
      if (d <0 ) {
        s = 1;
        d = -d;
      }
      if (val_count[s].size() <= d) {
        val_count[s].resize(d + 1);
      }
      val_count[s][d]++;
      head = x;
    }
    len++;
  }
  val_count[0].insert(val_count[0].end(),
                      val_count[1].begin(),
                      val_count[1].end());
  double e_len = entropy(len_count);
  double e_val = entropy(val_count[0]);
  double ent = e_len + e_val;
  std::cout << std::fixed << e_val << "+" << e_len << " = " << ent << "\n";
  std::cout << int(ent / 8) << " B\n";
}
