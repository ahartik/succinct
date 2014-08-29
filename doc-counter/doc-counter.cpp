#include "rle-wavelet.hpp"
#include "rrr-bit-vector.hpp"
#include "ilcp-count.hpp"
#include "brute-count.hpp"
#include "sada-count.hpp"
#include "sada-sparse-count.hpp"
#include "rle-ilcp-count.hpp"
#include "rle-bit-vector.hpp"
#include "delta-bit-vector.hpp"
#include "rle-delta-vector.hpp"

#include <gflags/gflags.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

DEFINE_string(structures, "sada_sparse",
              "Comma-separated list of tested structures. Available structures"
              ": brute,wt,wt_skewed,wt_rle,wt_rle_skewed,wt_rrr,wt_skewed_rrr,"
              "wt_rle_rrr,wt_rle_skewed_rrr,rle,sada,sada_rrr,sada_sparse"
              );

DEFINE_string(pattern_file, "",
              "File containing patterns to be counted, each on its own line.");

DEFINE_string(document_file, "",
              "File containing all the documents null-separated.");

DEFINE_bool(print_counts, false,
            "Print counts to standard output. For debugging purposes.");

const size_t kMB = 1024 * 1024;

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

const int kTotalCounts = 10000;

template<typename Counter> 
void countPatterns(const SuffixArray& sa,
                   const std::vector<std::string>& patterns) {
  using namespace std::chrono;
  std::vector<SuffixArray::SuffixRange> ranges;
  std::chrono::high_resolution_clock clock;

  ranges.reserve(patterns.size());
  {
    auto start = clock.now();
    for (const std::string& p : patterns) {
      ranges.push_back(sa.locate(p));
    }
    auto end = clock.now();
    std::cout << duration_cast<nanoseconds>(end-start).count()/patterns.size()<< "ns/locate\n";
  }
  auto cons_start = clock.now();
  Counter counter(sa);
  auto cons_end = clock.now();
  std::cout << duration_cast<milliseconds>(cons_end-cons_start).count()
            << "ms construction\n";
     
  uint64_t checksum = 0;
  std::vector<int> counts(patterns.size());
  auto start = clock.now();
  int cc = 0;
  while (cc < kTotalCounts) {
    for (size_t i = 0; i < patterns.size(); ++i) {
      const std::string& p = patterns[i];
      int c = counter.count(ranges[i], p);
      counts[i] = c;
      checksum = checksum * 31 + c;
      cc ++;
    }
  }
  auto end = clock.now();
  std::cout << duration_cast<nanoseconds>(end-start).count()/cc << "ns/count\n";
  
  if (FLAGS_print_counts) {
    for (size_t i = 0; i < patterns.size(); ++i) {
      const std::string& p = patterns[i];
      int c = counts[i];
      std::cout <<"\t" <<  p << ": " << c << "\n";
    }
  }

  size_t size = counter.byteSize();
  std::cout << "size: " << size << " B = " << (size / double(kMB)) << " MB\n";
  std::cout << "checksum: " << checksum << "\n";
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
    "usage: ./doc-counter --structures struct1,struct2 --pattern_file pattern "
    " --document_file documents"
      );
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  bool success = true;
  if (FLAGS_pattern_file.empty()) {
    std::cerr << "--pattern_file must be defined\n";
    success = false;
  }
  if (FLAGS_document_file.empty()) {
    std::cerr << "--document_file must be defined\n";
    success = false;
  }
  if (FLAGS_structures.empty()) {
    std::cerr << "--structures must be defined\n";
    success = false;
  }
  if (!success) {
    std::cerr << gflags::ProgramUsage() << std::endl;
    return 1;
  }
  // Read text
  std::vector<char> text;
  readBinaryFile(FLAGS_document_file, &text);
  SuffixArray sa(text.data(), text.size());
  // free memory.
  text = std::vector<char>(); 

  // Read patterns
  std::vector<std::string> patterns;
  std::ifstream pattern_in(FLAGS_pattern_file.c_str());
  for (std::string pattern; std::getline(pattern_in, pattern); ) {
    patterns.push_back(pattern);
  }

  std::unordered_map<std::string,
      std::function<void(const SuffixArray&, const std::vector<std::string>&)>>
    structFuncs;

  typedef RRRBitVector<63> RRR;
  structFuncs["brute"] = &countPatterns<BruteCount>;
  structFuncs["wt_skewed"] = &countPatterns<ILCPCount<
      SkewedWavelet<>>>;
  structFuncs["wt_balanced"] = &countPatterns<ILCPCount<
      BalancedWavelet<>>>;
  structFuncs["wt_rle"] = &countPatterns<ILCPCount<
      RLEWavelet<BalancedWavelet<>>>>;
  structFuncs["wt_rlebv"] = &countPatterns<ILCPCount<
      BalancedWavelet<RLEBitVector>>>;
  structFuncs["wt_rledv"] = &countPatterns<ILCPCount<
      BalancedWavelet<DeltaBitVector<RLEDeltaVector<>>>
      >>;
  structFuncs["wt_skewed_rledv"] = &countPatterns<ILCPCount<
      SkewedWavelet<DeltaBitVector<RLEDeltaVector<>>>
      >>;
  structFuncs["wt_skewed_rlebv"] = &countPatterns<ILCPCount<
      SkewedWavelet<RLEBitVector>>>;
  structFuncs["wt_rle_skewed"] = &countPatterns<ILCPCount<
      RLEWavelet<SkewedWavelet<>>>>;
  structFuncs["wt_skewed_rrr"] = &countPatterns<ILCPCount<
      SkewedWavelet<RRR>>>;
  structFuncs["wt_rrr"] = &countPatterns<ILCPCount<
      BalancedWavelet<RRR>>>;
  structFuncs["wt_rle_rrr"] = &countPatterns<ILCPCount<
      RLEWavelet<BalancedWavelet<RRR>>>>;
  structFuncs["wt_rle_skewed_rrr"] = &countPatterns<ILCPCount<
      RLEWavelet<SkewedWavelet<RRR>>>>;

  structFuncs["rle"] = &countPatterns<RLEILCPCount>;

  structFuncs["sada"] = &countPatterns<SadaCount<FastBitVector>>;
  structFuncs["sada_rrr"] = &countPatterns<SadaCount<RRR>>;
  structFuncs["sada_rle"] = &countPatterns<SadaCount<RLEBitVector>>;
  structFuncs["sada_rledv"] = &countPatterns<SadaCount<
      DeltaBitVector<RLEDeltaVector<>>
      >>;
  structFuncs["sada_sparse"] = &countPatterns<SadaSparseCount<1>>;
  structFuncs["sada_sparse_rrr"] = &countPatterns<SadaSparseCount<1, RRR>>;
  structFuncs["sada_sparse_rle"] = &countPatterns<SadaSparseCount<1,
      RLEBitVector>>;
  structFuncs["sada_sparse_simpler"] = &countPatterns<SadaSparseCount<0>>;
  
  for (const std::string& s : split(FLAGS_structures, ',')) {
    if (structFuncs.count(s)) {
      std::cout << "structure: " << s << "\n";
      structFuncs[s](sa, patterns);
    } else {
      std::cout << "unknown structure: " << s << "\n";
      return 1;
    }
    std::cout << "\n";
  }
}
