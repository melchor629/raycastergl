# Code generated from https://glad.dav1d.de/
cmake_minimum_required(VERSION 3.15)

project(GLAD C)

include_directories(include)

file(GLOB SOURCES src/*.c)

add_library(GLAD STATIC ${SOURCES})
target_include_directories(GLAD
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(GLAD -ldl)
