cmake_minimum_required(VERSION 3.10)
project(libml-feature CXX)
set(CMAKE_CXX_STANDARD 17)

set(FEATURE_DIR ../../include/feature)
include_directories(${FEATURE_DIR})
add_library(ml-feature ExtractFeatureExecutor.cc)
