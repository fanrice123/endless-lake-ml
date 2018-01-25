cmake_minimum_required(VERSION 3.10.1)
# set language standard
set(CMAKE_CXX_STANDARD 17)

# project name
project(endless_lake_ml CXX)

# version number
set(endless_lake_ml_VERSION 1)

# add library files
set(HEADER_DIR include)
include_directories(${HEADER_DIR})

# find X11
find_package(X11 REQUIRED)
# find opencv
find_package(OpenCV REQUIRED)
# find thread library
find_package(Threads REQUIRED)


include_directories(${X11_INCLUDE_DIR})

add_subdirectory(lib)
add_executable(collect collect.cc)
target_link_libraries(collect ${OpenCV_LIBS})
target_link_libraries(collect Threads::Threads)
target_link_libraries(collect ${X11_LIBRARIES})
target_link_libraries(collect -latomic)
target_link_libraries(collect ml)


#add_executable(train train.cc)
#target_link_libraries(train ${OpenCV_LIBS})
#target_link_libraries(train Threads::Threads)
#target_link_libraries(train ${X11_LIBRARIES})
#target_link_libraries(train -latomic)
#target_link_libraries(train ml)
