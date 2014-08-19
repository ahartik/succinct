#include <iostream>
#include <fstream>
#include <gflags/gflags.h>
#include "suffix-array.hpp"
#include "ilcp-common.hpp"

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

DEFINE_string(file, "", "Input document collection.");
DEFINE_bool(delta, false, "Write delta");
DEFINE_bool(analyze, false, "Only analyze");

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
  if (FLAGS_analyze) {
    int runs = 0;
    SuffixArray::Index last = -1;
    for (int x : ilcp) {
      if (x != last) runs++;
      last = x;
    }
    std::cout << runs << " runs\n";
    std::cout << runs * 8 << " bytes with simple rle compression\n";
  } else {
    std::string ofile(FLAGS_file + ".ilcp");
    if (FLAGS_delta) {
      ofile += ".delta";
      int last = 0;
      for (int& x : ilcp) {
        x = x - last;
        last += x;
      }
    }

    std::ofstream out(ofile);
    out.write(reinterpret_cast<char*>(&ilcp[0]),
              sizeof(SuffixArray::Index) * ilcp.size());
  }
}
