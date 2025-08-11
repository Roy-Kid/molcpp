cmake_minimum_required(VERSION 3.16)
project(molcpp_aabb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(MOLCPP_BUILD_PYTHON "Build Python extension" ON)
option(MOLCPP_ENABLE_OPENMP "Enable OpenMP" ON)
option(MOLCPP_BUILD_TEST "Build C++ test executable" ON)

# Dependencies (header-only): xtensor, xtensor-python, xsimd, pybind11
find_package(Python3 COMPONENTS Interpreter Development QUIET)
find_package(pybind11 CONFIG QUIET)

# Allow specifying custom include dirs via environment or cache.
set(MOLCPP_EXTERNAL_INCLUDE_DIRS "" CACHE STRING "Extra include directories for xtensor, xtensor-python, xsimd, pybind11")

# OpenMP
if(MOLCPP_ENABLE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found: enabling")
  else()
    message(STATUS "OpenMP not found: proceeding without OpenMP")
  endif()
endif()

# Compiler warnings and SIMD
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Sources
file(GLOB_RECURSE MOLCPP_HEADERS CONFIGURE_DEPENDS include/*.hpp)

# Always provide an interface library for includes
add_library(molcpp_aabb_headers INTERFACE)
target_include_directories(molcpp_aabb_headers INTERFACE include ${MOLCPP_EXTERNAL_INCLUDE_DIRS})

# Python module (optional)
if(MOLCPP_BUILD_PYTHON)
  if(Python3_Development_FOUND)
    set(MOLCPP_SOURCES src/bindings.cpp)
    add_library(molcpp_aabb MODULE ${MOLCPP_SOURCES} ${MOLCPP_HEADERS})
    target_include_directories(molcpp_aabb
      PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${MOLCPP_EXTERNAL_INCLUDE_DIRS}
        include
    )
    if (NOT MSVC)
      target_compile_options(molcpp_aabb PRIVATE -ffast-math)
    endif()
    if(OpenMP_CXX_FOUND)
      target_link_libraries(molcpp_aabb PRIVATE OpenMP::OpenMP_CXX)
    endif()
    if(pybind11_FOUND)
      target_link_libraries(molcpp_aabb PRIVATE pybind11::module)
    endif()
    set_target_properties(molcpp_aabb PROPERTIES PREFIX "" OUTPUT_NAME "molcpp_aabb")
    if(APPLE)
      set_target_properties(molcpp_aabb PROPERTIES SUFFIX ".so")
    endif()
    include(GNUInstallDirs)
    install(TARGETS molcpp_aabb LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  else()
    message(STATUS "Python development headers not found; skipping Python module")
  endif()
endif()

# Build test executable to validate compilation
if(MOLCPP_BUILD_TEST)
  add_executable(molcpp_aabb_test tests/test_build.cpp)
  target_include_directories(molcpp_aabb_test PRIVATE include ${MOLCPP_EXTERNAL_INCLUDE_DIRS})
  if(OpenMP_CXX_FOUND)
    target_link_libraries(molcpp_aabb_test PRIVATE OpenMP::OpenMP_CXX)
  endif()
endif()
