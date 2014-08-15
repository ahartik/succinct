cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(succinct CXX C)

include_directories(include)
include_directories(libdivsufsort-2.0.1/include)

add_subdirectory(libdivsufsort-2.0.1)
ADD_DEFINITIONS(
      -std=c++11
      -Wall
      -march=native
      -O3
       )

add_library(succinct lib/fast-bit-vector.cpp lib/bit-utils.cpp)

add_subdirectory(test)
add_subdirectory(benchmark)
