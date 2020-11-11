cmake_minimum_required(VERSION 3.11.4)
project(TinyRenderer)

set(CMAKE_CXX_STANDARD 17)

add_library(tga STATIC ext/tgaimage.cpp ext/tgaimage.h)
target_include_directories(tga PUBLIC ext)

add_library(model STATIC ext/model.cpp ext/model.h ext/geometry.h)
target_include_directories(model PUBLIC ext)

add_subdirectory(lesson-0)
add_subdirectory(lesson-1)
add_subdirectory(lesson-2)
add_subdirectory(lesson-3)

