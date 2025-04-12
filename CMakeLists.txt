cmake_minimum_required(VERSION 3.10)
project(ucm-gdc-s25)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(ucm-gdc-s25 src/main.cpp)
target_link_libraries(ucm-gdc-s25 ${SDL2_LIBRARIES})
