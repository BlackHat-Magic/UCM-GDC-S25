cmake_minimum_required(VERSION 3.10)
project(ucm-gdc-s25)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES src/*.cpp)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(ucm-gdc-s25 ${SOURCES})

target_link_libraries(ucm-gdc-s25
	SDL2
	SDL2_image
)
