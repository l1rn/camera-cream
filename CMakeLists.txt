cmake_minimum_required(VERSION 3.16)
project(OpenCV5_Test_Application CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

add_executable(opencv_app main.cpp)

target_include_directories(opencv_app PRIVATE /usr/local/include/opencv5)
target_link_libraries(opencv_app PRIVATE ${OpenCV_LIBS})