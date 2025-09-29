cmake_minimum_required(VERSION 4.0)

# Read version from core/VERSION
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/VERSION")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/core/VERSION" MOLCPP_FULL_VERSION)
    string(STRIP "${MOLCPP_FULL_VERSION}" MOLCPP_FULL_VERSION)
    string(REGEX REPLACE "([0-9]*)\\.([0-9]*)\\.([0-9]*).*" "\\1.\\2.\\3" MOLCPP_VERSION ${MOLCPP_FULL_VERSION})
else()
    set(MOLCPP_VERSION "0.1.0")
    set(MOLCPP_FULL_VERSION "0.1.0")
endif()

project(molcpp LANGUAGES C CXX VERSION "${MOLCPP_VERSION}")
set(PROJECT_VERSION ${MOLCPP_FULL_VERSION})

# Add cmake directory to module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
include(deps)

# Build options
option(MOLCPP_BUILD_CORE "Build molcpp core library" ON)
option(MOLCPP_BUILD_PYTHON "Build Python bindings" OFF)
option(MOLCPP_BUILD_TESTS "Build tests" ON)
option(MOLCPP_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(MOLCPP_USE_SIMD "Enable SIMD optimizations using xsimd" ON)
option(MOLCPP_BUILD_WASM "Build WASM xyz subset" OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_definitions(-DXTENSOR_ENABLE_XSIMD)
# add_definitions(-DXTENSOR_USE_XSIMD)\
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mavx2 -ffast-math")

message(STATUS "")
message(STATUS "molcpp v${MOLCPP_FULL_VERSION}")
message(STATUS "==========================================")
message(STATUS "Build options:")
message(STATUS "  MOLCPP_BUILD_CORE:        ${MOLCPP_BUILD_CORE}")
message(STATUS "  MOLCPP_BUILD_PYTHON:      ${MOLCPP_BUILD_PYTHON}")
message(STATUS "  MOLCPP_BUILD_TESTS:       ${MOLCPP_BUILD_TESTS}")
message(STATUS "  MOLCPP_BUILD_BENCHMARKS:  ${MOLCPP_BUILD_BENCHMARKS}")
message(STATUS "  MOLCPP_BUILD_WASM:        ${MOLCPP_BUILD_WASM}")
message(STATUS "  CMAKE_BUILD_TYPE:         ${CMAKE_BUILD_TYPE}")
message(STATUS "==========================================")
message(STATUS "")

# Setup dependencies based on what we're building
if(MOLCPP_BUILD_CORE OR MOLCPP_BUILD_PYTHON)
    find_or_fetch_xtensor()
endif()

if(MOLCPP_BUILD_WASM)
    # xtensor headers needed as well
    find_or_fetch_xtensor()
endif()

if(MOLCPP_BUILD_PYTHON)
    find_or_fetch_xtensor_python()
endif()

if(MOLCPP_BUILD_TESTS)
    find_or_fetch_catch2()
endif()

# Build core library
if(MOLCPP_BUILD_CORE)
    message(STATUS "Building molcpp core library")
    add_subdirectory(core)
endif()

# Build Python bindings
if(MOLCPP_BUILD_PYTHON)
    message(STATUS "Building molcpp Python bindings")
    add_subdirectory(python)
endif()

if(MOLCPP_BUILD_WASM)
    message(STATUS "Building molcpp WASM xyz subset")
    add_subdirectory(wasm)
endif()

# Build examples (disabled for now)
# if(MOLCPP_BUILD_CORE)
#     message(STATUS "Building molcpp examples")
#     add_subdirectory(examples)
# endif()
