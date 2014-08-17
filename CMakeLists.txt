cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(succinct CXX C)
include_directories(include)
# include_directories(libdivsufsort-2.0.1/include)

add_subdirectory(libdivsufsort)
ADD_DEFINITIONS(
      -std=c++11
      -march=native
      -Wall
      -O3
      -g
       )

file(GLOB LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(succinct ${LIB_SRC_FILES})

add_subdirectory(benchmark)
add_subdirectory(test)
