cmake_minimum_required(VERSION 3.5)
project(memory-allocators LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SOURCES
    src/CAllocator.cpp
    src/LinearAllocator.cpp
    src/StackAllocator.cpp
    src/PoolAllocator.cpp
    src/FreeListAllocator.cpp
    src/Benchmark.cpp
    src/main.cpp)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

target_compile_options(main PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
        -Wcast-align -Woverloaded-virtual -Wnull-dereference>
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:Debug>:-O0 -g -fsanitize=address,undefined>)

target_link_options(main PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
