cmake_minimum_required(VERSION 3.22)

project(json-wrapper-lib LANGUAGES CXX VERSION 0.0.1)

# C/C++
set(CMAKE_C_COMPILER "gcc-12")
set(CMAKE_CXX_COMPILER "g++-12")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# build mode specifics
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D_DEBUG)
  set(TARGET_DIR "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
  add_definitions(-DNDEBUG)
  set(TARGET_DIR "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${TARGET_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${TARGET_DIR}")

# find files
include_directories("${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE INC_HPP "include/*.hpp")
file(GLOB_RECURSE CPPSRC "src/*.cpp")

# reader library options
# building for rapidjson, smidjson, and NAPI
# could generate a header file to match the configuration

# writer library options

# static/shared library
add_library(json_wrapper_obj OBJECT ${INC_HPP} ${CPPSRC})
add_library(json_wrapper_static STATIC $<TARGET_OBJECTS:json_wrapper_obj>)
add_library(json_wrapper SHARED $<TARGET_OBJECTS:json_wrapper_obj>)

# tests
file(GLOB_RECURSE TESTSRC "tests/*.cpp" "tests/*.hpp")
add_executable(json_wrapper_tests ${TESTSRC} ${INC_HPP})
target_link_libraries(json_wrapper_tests json_wrapper_static)
