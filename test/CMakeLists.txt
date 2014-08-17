cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_subdirectory(gtest)

add_definitions(-g -O0)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(test ${TEST_SRC_FILES})

add_dependencies(test googletest)
target_link_libraries(test
  gtest
  gtest_main
  pthread
  succinct
  divsufsort
)

add_custom_command(TARGET test POST_BUILD COMMAND test || true)

