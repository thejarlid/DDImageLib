cmake_minimum_required(VERSION 2.8)
set(CMAKE_MACOSX_RPATH 1)

project(DDImageVideoLib)

set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always -std=c++11 -pthread -O2 -g -march=native -fPIC")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)


add_library(DDImgVidLib SHARED
  src/utils/utils.cpp
  src/utils/utils.h

  src/image/inc/stb_image.h
  src/image/inc/stb_image_write.h
  src/image/src/load_image.cpp
  src/image/src/image.cpp
  src/image/src/process_image.cpp
  src/image/src/resize_image.cpp
  src/image/src/filter_image.cpp

  src/feature_detection/harris_detector.cpp

  src/matrix/matrix.cpp
  
  src/panorama/panorama.cpp

  src/optical_flow/optical_flow.cpp


  src/colourspace/colourspaces.cpp
  src/colourspace/colourspaces.h
)

link_libraries(DDImgVidLib m stdc++)

add_executable(ProcessTests test/test_process_image.cpp)
add_executable(ResizeTests test/test_resize_image.cpp)
add_executable(FilterTests test/test_filter_image.cpp)
add_executable(HarrisTest test/test_harris.cpp)


find_package(OpenCV QUIET)

message(${OpenCV_LIBRARIES})
message(${OpenCV_INCLUDE_DIRS})
message(${OpenCV_FOUND})


if(${OpenCV_FOUND})
  add_definitions(-DOPENCV)
  link_libraries(${OpenCV_LIBRARIES})
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()
