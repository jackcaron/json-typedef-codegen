cmake_minimum_required(VERSION 3.22)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

project(json-wrapper-lib LANGUAGES CXX)

# options
option(ENABLE_SIMD_JSON "add SIMD JSON wrapper" OFF)
option(ENABLE_NLOH_JSON "add Nlohmann JSON wrapper" OFF)
option(ENABLE_NAPI "add Node.js' NAPI wrapper" OFF)
option(BUILD_READER "build the reader wrappers" ON)
option(BUILD_WRITER "build the reader wrappers" OFF)
option(BUILD_TEST "build the tests" ON)

# params
if (CMAKE_JS_VERSION)
  set(ENABLE_NAPI ON)
endif()

# option checks
if ((NOT BUILD_READER) AND (NOT BUILD_WRITER))
  message(FATAL_ERROR "option BUILD_READER or BUILD_WRITER must be set")
endif()

if ((NOT ENABLE_SIMD_JSON) AND (NOT ENABLE_NLOH_JSON) AND (NOT ENABLE_NAPI))
  message(FATAL_ERROR "option ENABLE_SIMD_JSON, ENABLE_NLOH_JSON, or ENABLE_NAPI must be set")
endif()

if (ENABLE_SIMD_JSON AND (NOT BUILD_READER))
  message(FATAL_ERROR "SIMD JSON wrapper can only be built if reader wrappers are built")
endif()

# C/C++
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# build mode specifics
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D_DEBUG)
  set(TARGET_DIR "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fconcepts-diagnostics-depth=3")
else()
  add_definitions(-DNDEBUG)
  set(TARGET_DIR "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${TARGET_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${TARGET_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${TARGET_DIR}")

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
    message(FATAL_ERROR "Cannot find 'node-addon-api':\n${ERR_NAPI}")
  endif()

  include_directories("${NAPI_INCL}")
endif()

if (ENABLE_SIMD_JSON)
  find_package(simdjson CONFIG REQUIRED)
endif()

if (ENABLE_NLOH_JSON)
  find_package(nlohmann_json CONFIG REQUIRED)
endif()

# find files
include_directories("${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE INC_HPP "include/*.hpp")
file(GLOB CPPSRC "src/*.cpp" "src/*.hpp" "src/stream_writer/*.cpp" "src/stream_writer/*.hpp")

# reader library options
if (BUILD_READER)

  if (ENABLE_SIMD_JSON)
    add_definitions(-DUSE_SIMD)
    file(GLOB SIMDJSONSRC "src/simd_json/*.cpp" "src/simd_json/*.hpp")
    set(CPPSRC ${CPPSRC} ${SIMDJSONSRC} ${SIMDJSONEXT})
  endif()

  if (ENABLE_NLOH_JSON)
    add_definitions(-DUSE_IN_NLOH)
    file(GLOB NLOHJSONSRC "src/nlohmann_reader/*.cpp" "src/nlohmann_reader/*.hpp")
    set(CPPSRC ${CPPSRC} ${NLOHJSONSRC})
  endif()

  if (ENABLE_NAPI)
    add_definitions(-DUSE_IN_NAPI)
    file(GLOB_RECURSE NAPISRC "src/napi_reader/*.cpp" "src/napi_reader/*.hpp")
    set(CPPSRC ${CPPSRC} ${NAPISRC})
  endif()

endif()

# writer library options
if (BUILD_WRITER)

  if (ENABLE_NLOH_JSON)
    add_definitions(-DUSE_OUT_NLOH)
    file(GLOB NLOHJSONSRC "src/nlohmann_writer/*.cpp" "src/nlohmann_writer/*.hpp")
    set(CPPSRC ${CPPSRC} ${NLOHJSONSRC})
  endif()

  if (ENABLE_NAPI)
    add_definitions(-DUSE_OUT_NAPI)
    file(GLOB_RECURSE NAPISRC "src/napi_writer/*.cpp" "src/napi_writer/*.hpp")
    set(CPPSRC ${CPPSRC} ${NAPISRC})
  endif()

endif()

# static/shared library
add_library(json_wrapper_obj OBJECT ${INC_HPP} ${CPPSRC})
if (ENABLE_NAPI)
  add_library(json_wrapper_napi_static STATIC $<TARGET_OBJECTS:json_wrapper_obj>)
else()
  add_library(json_wrapper_static STATIC $<TARGET_OBJECTS:json_wrapper_obj>)
  add_library(json_wrapper SHARED $<TARGET_OBJECTS:json_wrapper_obj>)
  set_target_properties(
    json_wrapper PROPERTIES
    VERSION "${VERSION}"
    SOVERSION "${VERSION_MAJOR}")
  if (simdjson_FOUND)
    target_link_libraries(json_wrapper simdjson::simdjson)
  endif()
  if (nlohmann_json_FOUND)
    target_link_libraries(json_wrapper nlohmann_json::nlohmann_json)
  endif()
endif()

# tests
if (BUILD_TEST)

  if (ENABLE_NAPI)
    if (CMAKE_JS_VERSION)
      file(GLOB_RECURSE TESTSRC "tests/napi_tests/*.cpp" "tests/napi_tests/*.hpp")

      add_library(json_wrapper_tests SHARED ${TESTSRC} ${INC_HPP})
      set_target_properties(json_wrapper_tests PROPERTIES PREFIX "" SUFFIX ".node")
      target_link_libraries(json_wrapper_tests json_wrapper_napi_static)
    else()
      message(FATAL_ERROR "need cmake-js to build the NAPI tests")
    endif()

  else() #else  ENABLE_NAPI

    # look for jtd-codegen binary
    get_filename_component(TARGET_BASE "${CMAKE_SOURCE_DIR}/../target" REALPATH)
    set(JTD_BIN "${TARGET_BASE}/release/jtd-codegen")
    if (NOT EXISTS "${JTD_BIN}")
      set(JTD_BIN "${TARGET_BASE}/debug/jtd-codegen")
      if (NOT EXISTS "${JTD_BIN}")
        message(FATAL_ERROR "jtd-codegen must be compiled")
      endif()
    endif()

    file(GLOB_RECURSE SCHEMAS "tests/gtests/schemas/*.jtd.json")
    get_filename_component(CFG_FILE "cpp_config.json" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}/tests/gtests")
    set(GEN_DIR "${CMAKE_SOURCE_DIR}/tests/gtests/generated")

    # generate temporary C++ files from schemas
    set(SCHM_HEADERS "")
    set(SCHM_SOURCE "")

    foreach(schm ${SCHEMAS})
      get_filename_component(baseName ${schm} NAME_WE)

      set(EXP_FILE_CPP "${GEN_DIR}/${baseName}.cpp")
      set(EXP_FILE_HPP "${GEN_DIR}/${baseName}.hpp")

      add_custom_command(
        OUTPUT "${EXP_FILE_CPP}" "${EXP_FILE_HPP}"
        COMMAND mkdir
        ARGS "-p" "${GEN_DIR}"
        COMMAND ${JTD_BIN}
        ARGS "${schm}" "--cpp-out" "${GEN_DIR}" "--cpp-props" "${CFG_FILE}"
        DEPENDS ${schm} ${JTD_BIN} ${CFG_FILE}
      )

      list(APPEND SCHM_HEADERS ${EXP_FILE_HPP})
      list(APPEND SCHM_SOURCE ${EXP_FILE_CPP})
    endforeach()

    find_package(GTest REQUIRED)
    file(GLOB TESTSRC "tests/gtests/*.cpp")
    file(GLOB TESTHPP "tests/gtests/*.hpp")

    # add test options based on the build options
    add_executable(json_wrapper_tests ${TESTSRC} ${TESTHPP} ${INC_HPP} ${SCHM_SOURCE} ${SCHM_HEADERS})
    target_link_libraries(json_wrapper_tests json_wrapper_static GTest::gtest GTest::gtest_main)
    if (simdjson_FOUND)
      target_link_libraries(json_wrapper_tests simdjson::simdjson)
    endif()
    if (nlohmann_json_FOUND)
      target_link_libraries(json_wrapper_tests nlohmann_json::nlohmann_json)
    endif()
  endif()

endif()
