cmake_minimum_required(VERSION 3.10)
project(libml CXX)
set(CMAKE_CXX_STANDARD 17)

set(HEADER_DIR ../include)
include_directories(${HEADER_DIR})
add_subdirectory(feature)
add_library(ml STATIC mlimage.cc
                      mldisplay.cc 
                      mlinput.cc 
                      mlscrcap.cc
                      mlnet.cc
)

target_link_libraries(ml ml-feature)
