cmake_minimum_required(VERSION 3.10.1)
# set language standard
set(CMAKE_CXX_STANDARD 17)

# project name
project(endless_lake_ml CXX)

# version number
set(endless_lake_ml_VERSION 1)

# add library files
set(HEADER_DIR src/include)
set(LIB_DIR src/lib)
include_directories(${HEADER_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# find libatomic
find_package(Atomic REQUIRED)
# find X11
find_package(X11 REQUIRED)
# find opencv
find_package(OpenCV REQUIRED)
# find thread library
find_package(Threads REQUIRED)
# find protobuf
find_package(Protobuf REQUIRED)
# find eigen3
find_package(Eigen3 REQUIRED)
# find gflags
find_package(GFlags REQUIRED)
# find caffe2
find_package(Caffe2 REQUIRED)
# find glog
#find_package(Glog REQUIRED)


include_directories(${X11_INCLUDE_DIR})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${GFLAGS_INCLUDE_DIR})
include_directories(/usr/include/glog)

add_subdirectory(${LIB_DIR})
add_executable(collect src/collect.cc)
target_link_libraries(collect ${OpenCV_LIBS})
target_link_libraries(collect Threads::Threads)
target_link_libraries(collect ${X11_LIBRARIES})
target_link_libraries(collect ${ATOMIC_LIBRARY})
target_link_libraries(collect ml)

add_executable(train src/train.cc)
target_link_libraries(train ${OpenCV_LIBS})
target_link_libraries(train Threads::Threads)
target_link_libraries(train ${X11_LIBRARIES})
target_link_libraries(train ${ATOMIC_LIBRARY})
target_link_libraries(train ml)
target_link_libraries(train ${GFLAGS_LIBRARY})
target_link_libraries(train glog)
target_link_libraries(train ${CAFFE2_LIBRARY})
target_link_libraries(train ${PROTOBUF_LIBRARY})

configure_file(${CMAKE_SOURCE_DIR}/src/setting.json 
               ${CMAKE_CURRENT_BINARY_DIR}/setting.json COPYONLY)
#target_link_libraries(train Eigen)

#add_executable(train train.cc)
#target_link_libraries(train ${OpenCV_LIBS})
#target_link_libraries(train Threads::Threads)
#target_link_libraries(train ${X11_LIBRARIES})
#target_link_libraries(train -latomic)
#target_link_libraries(train ml)
