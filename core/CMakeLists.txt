# Core library CMakeLists.txt
# This file is included from the root CMakeLists.txt

# Set up policies
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW) # OSX RPATH handling
endif()
if (POLICY CMP0054)
    # Only interpret ``if()`` arguments as variables or keywords when unquoted.
    cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0063)
    # Use of `<LANG>_VISIBILITY_PRESET` in OBJECT libraries
    cmake_policy(SET CMP0063 NEW)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" OFF)

option(MOLCPP_BUILD_TESTS "Build unit tests." OFF)
option(MOLCPP_BUILD_BENCHMARKS "Build performance benchmarks." OFF)
option(MOLCPP_BUILD_DOCUMENTATION "Build the documentation." OFF)
option(MOLCPP_USE_WARNINGS "Compile the code with warnings (default in debug mode)" OFF)
option(MOLCPP_USE_CLANG_TIDY "Compile the code with clang-tidy warnings" OFF)

option(MOLCPP_BUILD_DOCTESTS "Build documentation tests as well as unit tests." ON)

set(LIB_INSTALL_DIR "lib" CACHE PATH "Path relative to CMAKE_INSTALL_PREFIX where to install libraries")
set(BIN_INSTALL_DIR "bin" CACHE PATH "Path relative to CMAKE_INSTALL_PREFIX where to install DLL/binaries")
set(INCLUDE_INSTALL_DIR "include" CACHE PATH "Path relative to CMAKE_INSTALL_PREFIX where to install headers")


mark_as_advanced(MOLCPP_BUILD_DOCTESTS)

# Set a default build type if none was specified
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    if("${CMAKE_BUILD_TYPE}" STREQUAL "" AND "${CMAKE_CONFIGURATION_TYPES}" STREQUAL "")
        message(STATUS "Setting build type to 'release' as none was specified.")
        set(CMAKE_BUILD_TYPE "release"
            CACHE STRING
            "Choose the type of build, options are: none(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) debug release relwithdebinfo minsizerel."
        FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS release debug relwithdebuginfo minsizerel none)
    endif()
endif()

include(CompilerFlags)
include(ClangTidy)

if(${MOLCPP_USE_WARNINGS})
    # Make sure to have asserts compiled
    add_definitions("-UNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MOLCPP_CXX_WARNINGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MOLCPP_C_WARNINGS}")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MOLCPP_CXX_WARNINGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${MOLCPP_C_WARNINGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MOLCPP_SANITIZERS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MOLCPP_SANITIZERS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MOLCPP_SANITIZERS}")


# We need to use a separated library for non-dll-exported classes that have an
# unit test. The molcpp_objects OBJECT library contains the code for files
# implementation and is linked into both the main molcpp library and the
# unit tests.
set(molcpp_objects_sources
    src/spatial/box.cpp
    src/spatial/boundary/open_boundary.cpp
    src/spatial/boundary/periodic_boundary.cpp
    src/spatial/region/parallelepiped_region.cpp
    src/spatial/region/cube_region.cpp
    src/spatial/region/sphere_region.cpp
    src/spatial/region/cylinder_region.cpp
    src/spatial/region/plane_region.cpp
    src/spatial/region/intersection_region.cpp
    src/spatial/region/union_region.cpp
    src/spatial/region/complement_region.cpp
)

add_library(molcpp_objects OBJECT ${molcpp_objects_sources})
target_compile_features(molcpp_objects PRIVATE cxx_std_20 c_std_99)
setup_xtensor(molcpp_objects)

target_include_directories(molcpp_objects SYSTEM BEFORE PRIVATE ${EXTERNAL_INCLUDES})

target_include_directories(molcpp_objects PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_include_directories(molcpp_objects SYSTEM BEFORE PRIVATE ${EXTERNAL_INCLUDES})

target_compile_definitions(molcpp_objects PRIVATE molcpp_EXPORTS)
if(NOT ${MOLCPP_SYSTEM_LZMA})
    target_compile_definitions(molcpp_objects PRIVATE LZMA_API_STATIC)
endif()

if(${BUILD_SHARED_LIBS})
    set_target_properties(molcpp_objects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()

# Always set POSITION_INDEPENDENT_CODE for Python bindings compatibility
# Python extension modules require PIC even when linking against static libraries
set_target_properties(molcpp_objects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# hide non-exported symbols by default, this mimics Windows behavior on Unix
set_target_properties(molcpp_objects PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Multiple different flags are required by different platform for 64-bit support
# These flags are also set in external libraries (zlib & lzma), but they might
# not be carried over when using system libraries.

# Default flag:
target_compile_definitions(molcpp_objects PRIVATE -D_FILE_OFFSET_BITS=64)
# Required for zlib 64-bit support:
target_compile_definitions(molcpp_objects PRIVATE -D_LARGEFILE64_SOURCE=1)
target_compile_definitions(molcpp_objects PRIVATE -D_LFS64_LARGEFILE=1)

if(MSVC)
    target_compile_definitions(molcpp_objects PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(molcpp_objects PRIVATE -D_SCL_SECURE_NO_WARNINGS)
endif()

if(WIN32)
    target_compile_definitions(molcpp_objects PRIVATE -DWIN32_LEAN_AND_MEAN)
    target_compile_definitions(molcpp_objects PRIVATE -DNOMINMAX)
endif()

target_use_clang_tidy(molcpp_objects)

set(MOLCPP_OBJECTS
    $<TARGET_OBJECTS:molcpp_objects>
)

# Add the main molcpp library
add_library(molcpp ${MOLCPP_OBJECTS})

target_compile_features(molcpp PUBLIC cxx_std_20 c_std_99)

set_target_properties(molcpp PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

set_target_properties(molcpp PROPERTIES
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
    # This needs to be the part of the version which matter for backward
    # compatibility, so "major.minor" until 1.0 and then "major" only
    SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
)

target_include_directories(molcpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


if(WIN32)
    # MMTF (and thus molcpp) uses endianness conversion function from ws2_32
    target_link_libraries(molcpp PUBLIC ws2_32)
    set(MOLCPP_WINDOWS ON)
endif()

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/include/molcpp/config.in.h"
  "${PROJECT_BINARY_DIR}/include/molcpp/config.h"
)

include(GenerateExportHeader)
generate_export_header(molcpp
    BASE_NAME MOLCPP
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/molcpp/exports.h
)

if(${MOLCPP_BUILD_DOCUMENTATION})
    add_subdirectory(doc)
endif()

if(${MOLCPP_BUILD_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

if(${MOLCPP_BUILD_BENCHMARKS})
    add_subdirectory(benchmarks)
endif()

#-------------------------------------------------------------------------#
# Installation configuration
#-------------------------------------------------------------------------#
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/molcpp-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(TARGETS molcpp
    EXPORT molcpp-targets
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/molcpp-config.cmake.in"
    "${PROJECT_BINARY_DIR}/molcpp-config.cmake"
    INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/molcpp
)

install(EXPORT molcpp-targets DESTINATION ${LIB_INSTALL_DIR}/cmake/molcpp)
install(
    FILES ${PROJECT_BINARY_DIR}/molcpp-config-version.cmake
          ${PROJECT_BINARY_DIR}/molcpp-config.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/molcpp
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/core/include/ DESTINATION ${INCLUDE_INSTALL_DIR})
# install(DIRECTORY ${PROJECT_BINARY_DIR}/core/include/ DESTINATION ${INCLUDE_INSTALL_DIR})

# uninstall target
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/uninstall.in.cmake"
    "${PROJECT_BINARY_DIR}/uninstall.cmake"
    @ONLY
)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/uninstall.cmake
)