cmake_minimum_required (VERSION 3.1)
project (SuffixTree)

# The version number.
set (SuffixTree_VERSION_MAJOR 0)
set (SuffixTree_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/SuffixTreeConfig.h.in"
  "${PROJECT_BINARY_DIR}/SuffixTreeConfig.h"
)
 
# add the binary tree to the search path for include files
# so that we will find SuffixTreeConfig.h
include_directories(SuffixTree PUBLIC
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
)


add_executable(SuffixTree src/string_substrings.cpp)