cmake_minimum_required(VERSION 3.16)

project(molcpp VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

# CMake helpers
# include(CMakePrintHelpers)
# cmake_print_variables(CMAKE_MODULE_PATH)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add dependencies
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/libs)
add_subdirectory(${CMAKE_MODULE_PATH}/chemfiles)

add_library(molcpp)

target_sources(
  molcpp
  PRIVATE
  src/graph.cpp
  src/atom.cpp
  src/bond.cpp
)

target_include_directories(
  molcpp
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  molcpp
  PUBLIC
  chemfiles
)

# For debug and test
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  enable_testing()
  add_subdirectory(tests)
  
endif()
