cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(doc-counter CXX C)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

add_executable(doc-counter doc-counter.cpp)
add_dependencies(doc-counter gflags libdivsufsort)

target_link_libraries(doc-counter
  succinct
  ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.a
  ${GFLAGS_LIBS_DIR}/libgflags.a
  pthread
)
