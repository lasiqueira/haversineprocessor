cmake_minimum_required(VERSION 3.16)
project(HaversineProcessor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set optimization level if not set by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set optimization flags for all platforms/compilers
if(MSVC)
    # /O2 is the highest optimization for MSVC
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    # Use -O3 if available, otherwise fallback to -O2
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Add -march=native for Clang and GCC (not MSVC)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-march=native)
endif()

# Add source files
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(HaversineProcessor ${SOURCES})

# Include directories
target_include_directories(HaversineProcessor PRIVATE src)

# Extra warnings for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(HaversineProcessor PRIVATE -Wall -Wextra -Wpedantic)
endif()