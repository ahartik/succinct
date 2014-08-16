cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_definitions(-O3 -DNDEBUG -march=native)
include_directories(../include)

add_executable(bit-vector_benchmark bit-vector_benchmark.cpp)
target_link_libraries(bit-vector_benchmark
  succinct
  )

add_executable(wavelet_benchmark wavelet_benchmark.cpp)
target_link_libraries(wavelet_benchmark
  succinct
  )

add_executable(csa_benchmark csa_benchmark.cpp)
target_link_libraries(csa_benchmark
  succinct
  divsufsort
  )
