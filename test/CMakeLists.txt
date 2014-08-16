cmake_minimum_required(VERSION 2.6.0)	
## Project information ##
project(succinct_test CXX C)

add_definitions(-g -O0)

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

add_executable(delta-vector_test delta-vector_test.cpp)
target_link_libraries(delta-vector_test
    succinct
    gtest_main
    gtest
    pthread
    )

add_executable(csa_test csa_test.cpp)
target_link_libraries(csa_test
    divsufsort
    succinct
    gtest_main
    gtest
    pthread
    )

add_executable(int-array_test int-array_test.cpp)
target_link_libraries(int-array_test
    succinct
    )

add_custom_command(TARGET csa_test POST_BUILD
    COMMAND csa_test || true)
  # add_executable(wt-csa_test wt-csa_test.cpp)
  # target_link_libraries(wt-csa_test
  #     divsufsort
  #     succinct
  #     )
