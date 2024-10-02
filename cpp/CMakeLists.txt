cmake_minimum_required(VERSION 3.22)

project(json-wrapper-lib LANGUAGES CXX VERSION 0.0.1)

# options
option(ENABLE_SIMD_JSON "add SIMD JSON wrapper" OFF)
option(ENABLE_RAPID_JSON "add Rapid JSON wrapper" OFF)
option(ENABLE_NAPI "add Node.js' NAPI wrapper" OFF)
option(BUILD_READER "build the reader wrappers" ON)
option(BUILD_WRITER "build the reader wrappers" OFF)

if ((NOT BUILD_READER) AND (NOT BUILD_WRITER))
  message(FATAL_ERROR "option BUILD_READER or BUILD_WRITER must be set")
endif()

if ((NOT ENABLE_SIMD_JSON) AND (NOT ENABLE_RAPID_JSON) AND (NOT ENABLE_NAPI))
  message(FATAL_ERROR "option ENABLE_SIMD_JSON, ENABLE_RAPID_JSON, or ENABLE_NAPI must be set")
endif()

if (ENABLE_SIMD_JSON AND (NOT BUILD_READER))
  message(FATAL_ERROR "SIMD JSON wrapper can only be built if reader wrappers are built")
endif()

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
file(GLOB INC_HPP "include/*.hpp")
file(GLOB CPPSRC "src/*.cpp" "src/*.hpp")

# reader library options
if (ENABLE_SIMD_JSON)
  file(GLOB SIMDJSONEXT "external/simd_json/simdjson.*")
  file(GLOB_RECURSE SIMDJSONSRC "src/simd_json/*.cpp" "src/simd_json/*.hpp")
  set(CPPSRC ${CPPSRC} ${SIMDJSONSRC} ${SIMDJSONEXT})
endif()

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
