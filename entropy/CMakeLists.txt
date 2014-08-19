cmake_minimum_required(VERSION 2.6.0)	
	
## Project information ##
project(entropy CXX C)

add_executable(raw-ilcp raw-ilcp.cpp)
add_dependencies(raw-ilcp gflags libdivsufsort)

target_link_libraries(raw-ilcp
  succinct
  ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.a
  ${GFLAGS_LIBS_DIR}/libgflags.a
  pthread
)

add_executable(ilcp-entropy ilcp-entropy.cpp)
add_dependencies(ilcp-entropy gflags libdivsufsort)

target_link_libraries(ilcp-entropy
  succinct
  ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.a
  ${GFLAGS_LIBS_DIR}/libgflags.a
  pthread
)
