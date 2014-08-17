cmake_minimum_required(VERSION 2.8.8)
project(libdivsufsort_builder C CXX)
include(ExternalProject)

ExternalProject_Add(libdivsufsort
    SVN_REPOSITORY http://libdivsufsort.googlecode.com/svn/trunk
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    CMAKE_ARGS -DBUILD_SHARED_LIBS=off
# Disable install step
    INSTALL_COMMAND ""
)

# # Specify include dir
# ExternalProject_Get_Property(libdivsufsort install_dir)

# Specify MainTest's link libraries
ExternalProject_Get_Property(libdivsufsort binary_dir)
set(LIBDIVSUFSORT_INCLUDE_DIRS ${binary_dir}/include PARENT_SCOPE)
set(LIBDIVSUFSORT_LIBS_DIR ${binary_dir}/lib PARENT_SCOPE)
