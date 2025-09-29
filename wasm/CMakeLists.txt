# CMake build for molcpp WASM (xyz frame parsing subset)
cmake_minimum_required(VERSION 4.0)

# Build this as part of the main molcpp project:
#   mkdir build-wasm && cd build-wasm
#   emcmake cmake .. -DMOLCPP_BUILD_WASM_XYZ=ON

project(molcpp_wasm_xyz LANGUAGES CXX)

# Bring in dependency helpers from root
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
include(deps)

# We need xtensor only for WASM
find_or_fetch_xtensor()

# set(MOLCPP_WASM_CORE_SOURCES
#     ${CMAKE_SOURCE_DIR}/core/src/io/utils.cpp
# 
# )

# Minimal IO-only sources (no NeighborList, no spatial/BLAS)
set(MOLCPP_WASM_IO_SOURCES
    ${CMAKE_SOURCE_DIR}/core/src/io/utils.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_parse.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_frame_reader.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_traj_reader.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_writer.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_utils.cpp
    ${CMAKE_SOURCE_DIR}/core/src/io/xyz/xyz_iter.cpp
)

# Build a small object library containing just the IO pieces and headers
add_library(molcpp_wasm_io OBJECT ${MOLCPP_WASM_IO_SOURCES})

target_include_directories(molcpp_wasm_io PUBLIC
    ${CMAKE_SOURCE_DIR}/core/include
)

target_compile_features(molcpp_wasm_io PRIVATE cxx_std_23)

# Setup xtensor only for WASM (no xsimd, no BLAS)
find_or_fetch_xtensor()
target_link_libraries(molcpp_wasm_io PUBLIC xtensor)

# Create WASM bindings (separate object lib)
add_library(molcpp_wasm_bindings OBJECT bindings_xyz.cpp converters.cpp)

target_include_directories(molcpp_wasm_bindings PRIVATE
    ${CMAKE_SOURCE_DIR}/core/include
)

target_compile_features(molcpp_wasm_bindings PRIVATE cxx_std_23)

# Setup xtensor for bindings as well (no xsimd, no BLAS)
find_or_fetch_xtensor()
target_link_libraries(molcpp_wasm_bindings PUBLIC xtensor)

# Final executable links both object libraries
add_executable(molcpp_xyz_wasm
    $<TARGET_OBJECTS:molcpp_wasm_io>
    $<TARGET_OBJECTS:molcpp_wasm_bindings>
)

# Emscripten flags
set(EXPORT_NAME "MolcppXYZ")
set(EMBIND_FLAGS
    -sMODULARIZE=1
    -sEXPORT_NAME=${EXPORT_NAME}
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web,worker,node
    -sFILESYSTEM=0
    -sDISABLE_EXCEPTION_CATCHING=0
    -lembind
)

foreach(t molcpp_wasm_io molcpp_wasm_bindings molcpp_xyz_wasm)
    target_compile_options(${t} PRIVATE -fexceptions)
    target_link_options(${t} PRIVATE ${EMBIND_FLAGS})
endforeach()

# Output directory
set_target_properties(molcpp_xyz_wasm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist
    OUTPUT_NAME molcpp_xyz
)
