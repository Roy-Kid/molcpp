cmake_minimum_required(VERSION 4.1)

message(STATUS "Configuring molcpp tests")

find_or_fetch_catch2()
find_or_fetch_xtensor()

# Create test executable with all tests - FOCUSED ON CORE MODULES
set(test_sources
    # Core module tests
    "test_core/test_block.cpp"
    "test_core/test_frame.cpp"
    "test_core/test_element.cpp"
    "test_core/test_bond.cpp"
    
    # Param system tests
    "test_core/test_param_system.cpp"
    "test_core/test_param_value.cpp"
    "test_core/test_value_ref.cpp"
    "test_core/test_param_list.cpp"
    "test_core/test_param_map.cpp"
    "test_core/test_param_container.cpp"
    
    # Core ECS tests - DISABLED (files not found)
    # "test_core/ecs/test_entity.cpp"
    # "test_core/ecs/test_system.cpp"
    
    # Core ForceField tests - TEMPORARILY DISABLED DUE TO COMPATIBILITY ISSUES
    # "test_core/forcefield/test_forcefield.cpp"
    
    # Locality module tests
    "test_locality/test_aabb_query.cpp"
    "test_locality/test_aabb_simple.cpp"
    "test_locality/test_neighbor_bond.cpp"
    "test_locality/test_neighbor_list.cpp"
    
    # Pack module tests
    "test_pack/test_constraint.cpp"
    "test_pack/test_optimizer.cpp"
    "test_pack/test_packer.cpp"
    "test_pack/test_target.cpp"
    
    # Spatial module tests
    "test_spatial/test_boundary.cpp"
    "test_spatial/test_box_boundary.cpp"
    "test_spatial/test_regions.cpp"
    
    # I/O tests
    "test_io/test_xyz.cpp"
)

add_executable(molcpp_tests ${test_sources})

# Setup dependencies individually
find_or_fetch_xtensor()
find_or_fetch_xsimd()
target_link_libraries(molcpp_tests PRIVATE xtensor xsimd)

# Add BLAS support for non-WASM builds
if(NOT EMSCRIPTEN)
    find_or_fetch_xtensor_blas()
    target_link_libraries(molcpp_tests PRIVATE xtensor-blas)
    target_compile_definitions(molcpp_tests PRIVATE HAVE_CBLAS=1)
    
    if(WIN32)
        find_package(OpenBLAS REQUIRED)
        target_link_libraries(molcpp_tests PRIVATE OpenBLAS::OpenBLAS)
    else()
        find_package(BLAS REQUIRED)
        find_package(LAPACK REQUIRED)
        target_link_libraries(molcpp_tests PRIVATE ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
    endif()
endif()

target_link_libraries(molcpp_tests PRIVATE 
    molcpp_objects
    Catch2::Catch2WithMain
)

target_include_directories(molcpp_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/core/include
)

# Setup common Catch2 target properties
setup_catch2_target(molcpp_tests)

# Register tests with CTest
catch_discover_tests(molcpp_tests)
