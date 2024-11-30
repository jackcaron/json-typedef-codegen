cmake_minimum_required(VERSION 3.22)

project(json-wrapper-lib LANGUAGES CXX VERSION 0.0.1)

# options
option(ENABLE_SIMD_JSON "add SIMD JSON wrapper" OFF)
option(ENABLE_RAPID_JSON "add Rapid JSON wrapper" OFF)
option(ENABLE_NAPI "add Node.js' NAPI wrapper" OFF)
option(BUILD_READER "build the reader wrappers" ON)
option(BUILD_WRITER "build the reader wrappers" OFF)
option(BUILD_TEST "build the tests" ON)

if (CMAKE_JS_VERSION)
  set(ENABLE_NAPI ON)
endif()

# option checks
if ((NOT BUILD_READER) AND (NOT BUILD_WRITER))
  message(FATAL_ERROR "option BUILD_READER or BUILD_WRITER must be set")
endif()

if ((NOT ENABLE_SIMD_JSON) AND (NOT ENABLE_RAPID_JSON) AND (NOT ENABLE_NAPI))
  message(FATAL_ERROR "option ENABLE_SIMD_JSON, ENABLE_RAPID_JSON, or ENABLE_NAPI must be set")
endif()

if (ENABLE_SIMD_JSON AND (NOT BUILD_READER))
  message(FATAL_ERROR "SIMD JSON wrapper can only be built if reader wrappers are built")
endif()

if (BUILD_READER)
  if (ENABLE_RAPID_JSON)
    message(FATAL_ERROR "option ENABLE_RAPID_JSON not supported yet")
  endif()
endif()

if (BUILD_WRITER)
  message(FATAL_ERROR "option BUILD_WRITER not supported yet")
endif()

# C/C++
set(CMAKE_C_COMPILER "gcc-13")
set(CMAKE_CXX_COMPILER "g++-13")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# build mode specifics
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D_DEBUG)
  set(TARGET_DIR "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fconcepts-diagnostics-depth=3")
else()
  add_definitions(-DNDEBUG)
  set(TARGET_DIR "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${TARGET_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${TARGET_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${TARGET_DIR}")

# find files
include_directories("${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE INC_HPP "include/*.hpp")
file(GLOB CPPSRC "src/*.cpp" "src/*.hpp")

# NAPI, use node.js/npm to find the include directories
if (ENABLE_NAPI)
  find_program(NODE_exec node REQUIRED)
  find_program(NPM_exec npm REQUIRED)
  add_definitions(-DNAPI_DISABLE_CPP_EXCEPTIONS)

  execute_process(
    COMMAND ${NPM_exec} "config" "get" "prefix"
    OUTPUT_VARIABLE NODE_PREFIX
    ERROR_VARIABLE ERR_NODE
  )

  if (NOT "${ERR_NODE}" STREQUAL "")
    message(FATAL_ERROR "Cannot find node.js base directory:\n${ERR_NODE}")
  endif()

  string(STRIP "${NODE_PREFIX}" NODE_PREFIX)
  include_directories("${NODE_PREFIX}/include/node")

  execute_process(
    COMMAND ${NODE_exec} "-p" "require('path').resolve(require('node-addon-api').include_dir)"
    OUTPUT_VARIABLE NAPI_INCL
    ERROR_VARIABLE ERR_NAPI
  )

  if (NOT "${ERR_NAPI}" STREQUAL "")
    message(FATAL_ERROR "Cannot find 'node-addon-api':\n${ERR_NAPIs}")
  endif()

  include_directories("${NAPI_INCL}")
endif()

# reader library options
if (BUILD_READER)

  if (ENABLE_SIMD_JSON)
    add_definitions(-DUSE_SIMD)
    include_directories("external")
    file(GLOB SIMDJSONEXT "external/simd_json/simdjson.cpp")
    file(GLOB SIMDJSONSRC "src/simd_json/*.cpp" "src/simd_json/*.hpp")
    set(CPPSRC ${CPPSRC} ${SIMDJSONSRC} ${SIMDJSONEXT})
  endif()

  if (ENABLE_RAPID_JSON)
    add_definitions(-DUSE_IN_RAPID)
    # LATER
  endif()

  if (ENABLE_NAPI)
    add_definitions(-DUSE_IN_NAPI)
    file(GLOB_RECURSE NAPISRC "src/napi/*.cpp" "src/napi/*.hpp")
    set(CPPSRC ${CPPSRC} ${NAPISRC})
  endif()

endif()

# writer library options
if (BUILD_WRITER)
  # LATER
endif()

# static/shared library
add_library(json_wrapper_obj OBJECT ${INC_HPP} ${CPPSRC})
add_library(json_wrapper_static STATIC $<TARGET_OBJECTS:json_wrapper_obj>)
if (NOT ENABLE_NAPI)
  add_library(json_wrapper SHARED $<TARGET_OBJECTS:json_wrapper_obj>)
endif()

# tests
if (BUILD_TEST)
  if (ENABLE_NAPI)
    if (CMAKE_JS_VERSION)
      file(GLOB_RECURSE TESTSRC "tests/napi_tests/*.cpp" "tests/napi_tests/*.hpp")

      add_library(json_wrapper_tests SHARED ${TESTSRC} ${INC_HPP})
      set_target_properties(json_wrapper_tests PROPERTIES PREFIX "" SUFFIX ".node")
      target_link_libraries(json_wrapper_tests json_wrapper_static)
    else()
      message(FATAL_ERROR "need cmake-js to build the NAPI tests")
    endif()
  else() # ENABLE_NAPI
    find_package(GTest)
    file(GLOB_RECURSE TESTSRC "tests/gtests/*.cpp" "tests/gtests/*.hpp")

    # add test options based on the build options
    add_executable(json_wrapper_tests ${TESTSRC} ${INC_HPP})
    target_link_libraries(json_wrapper_tests json_wrapper_static GTest::gtest GTest::gtest_main)
  endif()
endif()
