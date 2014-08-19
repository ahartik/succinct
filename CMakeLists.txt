cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(succinct CXX C)
# include_directories(libdivsufsort-2.0.1/include)

add_subdirectory(ext/libdivsufsort)
add_subdirectory(ext/gflags)
add_subdirectory(ext/gtest)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${GTEST_INCLUDE_DIRS}
  ${LIBDIVSUFSORT_INCLUDE_DIRS}
  ${GFLAGS_INCLUDE_DIRS}
  )

ADD_DEFINITIONS(
      -std=c++11
      -march=native
      -Wall
      -O3
       )

file(GLOB LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(succinct ${LIB_SRC_FILES})
add_dependencies(succinct libdivsufsort)

add_subdirectory(benchmark)
add_subdirectory(doc-counter)
add_subdirectory(entropy)
add_subdirectory(test)
