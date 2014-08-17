cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_definitions(-g -O0 -Wall)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(test ${TEST_SRC_FILES})
add_dependencies(test googletest libdivsufsort)
target_link_libraries(test
  ${GTEST_LIBS_DIR}/libgtest_main.a
  ${GTEST_LIBS_DIR}/libgtest.a
  ${LIBDIVSUFSORT_LIBS_DIR}/libdivsufsort.a
  pthread
  succinct
)

add_custom_command(TARGET test POST_BUILD COMMAND test || true)

