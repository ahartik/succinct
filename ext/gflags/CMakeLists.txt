cmake_minimum_required(VERSION 2.8.7)
project(gflags_builder C CXX)
include(ExternalProject)

ExternalProject_Add(gflags
  GIT_REPOSITORY https://code.google.com/p/gflags/
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
# Disable install step
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(gflags binary_dir)
set(GFLAGS_INCLUDE_DIRS ${binary_dir}/include PARENT_SCOPE)
set(GFLAGS_LIBS_DIR ${binary_dir}/lib/ PARENT_SCOPE)
