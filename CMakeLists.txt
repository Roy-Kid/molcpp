cmake_minimum_required(VERSION 4.1)
project(molcpp LANGUAGES C CXX)

# Options to enable/disable language bindings
option(MOLCPP_ENABLE_CORE     "Build molcpp core library" ON)

# Set global default visibility (optional)
set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(core)
