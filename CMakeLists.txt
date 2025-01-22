cmake_minimum_required(VERSION 3.10)

project(HaversineProcessor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cpp")

add_executable(HaversineProcessor ${SOURCES})

target_include_directories(HaversineProcessor PRIVATE ${CMAKE_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -mavx512f")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()