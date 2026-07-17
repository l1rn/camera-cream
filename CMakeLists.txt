cmake_minimum_required(VERSION 3.16)
project(camera-cream CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(version.cmake)

add_executable(camera-cream
    main.cpp
    src/StreamManager.cpp
    src/StreamServer.cpp
    src/V4L2Camera.cpp
)

target_include_directories(camera-cream PRIVATE 
    include
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)