cmake_minimum_required(VERSION 2.8.8)
project(libdivsufsort_builder C CXX)
include(ExternalProject)

ExternalProject_Add(libdivsufsort
    SVN_REPOSITORY http://libdivsufsort.googlecode.com/svn/trunk
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
# Disable install step
    INSTALL_COMMAND ""
)

# # Specify include dir
# ExternalProject_Get_Property(libdivsufsort source_dir)
# set(GTEST_INCLUDE_DIRS ${source_dir}/include PARENT_SCOPE)
# 
# # Specify MainTest's link libraries
# ExternalProject_Get_Property(googletest binary_dir)
# set(GTEST_LIBS_DIR ${binary_dir} PARENT_SCOPE)
# 
# 
# 
