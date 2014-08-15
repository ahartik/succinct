cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_definitions(-O3)
include_directories(../include)

add_executable(fast-bit-vector_benchmark fast-bit-vector_benchmark)
target_link_libraries(fast-bit-vector_benchmark
  succinct
  )

add_executable(wavelet_benchmark wavelet_benchmark.cpp)
target_link_libraries(wavelet_benchmark
  succinct
  )
