cmake_minimum_required(VERSION 3.11.4)
project(TinyRenderer)

set(CMAKE_CXX_STANDARD 17)

add_library(compiler-warnings INTERFACE)

# Determine whether we're compiling with clang++
string(FIND "${CMAKE_CXX_COMPILER}" "clang++" TINY_RENDERER_COMPILER_CLANGPP)
if(TINY_RENDERER_COMPILER_CLANGPP GREATER -1)
  set(TINY_RENDERER_COMPILER_CLANGPP 1)
else()
  set(TINY_RENDERER_COMPILER_CLANGPP 0)
endif()

target_compile_options(compiler-warnings
        INTERFACE
        $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>,${TINY_RENDERER_COMPILER_CLANGPP}>:
        -Wall
        -Wextra
        -pedantic-errors
        -Wconversion
        -Wsign-conversion>
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
        )


add_library(tga STATIC ext/tgaimage.cpp ext/tgaimage.h)
target_include_directories(tga PUBLIC ext)
target_link_libraries(tga INTERFACE compiler-warnings)

add_library(model STATIC ext/model.cpp ext/model.h ext/geometry.h)
target_include_directories(model PUBLIC ext)

add_subdirectory(lesson-0)
add_subdirectory(lesson-1)
add_subdirectory(lesson-2)
add_subdirectory(lesson-3)
add_subdirectory(lesson-4-5)
add_subdirectory(lesson-6)


