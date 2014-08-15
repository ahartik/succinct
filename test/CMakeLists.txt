cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_definitions(-g -O0)
include_directories(../include)

add_executable(bit-vector_test bit-vector_test.cpp)
target_link_libraries(bit-vector_test
  succinct
  libgtest.a
  libgtest_main.a
  pthread
  )
add_custom_command(TARGET bit-vector_test POST_BUILD
    COMMAND bit-vector_test || true)

add_executable(wavelet_test wavelet_test.cpp)
target_link_libraries(wavelet_test
  succinct
  libgtest.a
  libgtest_main.a
  pthread
  )
add_custom_command(TARGET wavelet_test POST_BUILD
    COMMAND wavelet_test || true)
