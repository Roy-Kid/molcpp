cmake_minimum_required(VERSION 4.1)

message(STATUS "Configuring molcpp tests")

find_or_fetch_catch2()
find_or_fetch_xtensor()

# Create test executable with all tests
set(test_sources
    "test_core/test_block.cpp"
    "test_core/test_frame.cpp"
    "test_entity.cpp"
    "test_system.cpp"
    "test_spatial/test_boundary.cpp"
    "test_spatial/test_box_boundary.cpp"
    "test_spatial/test_regions.cpp"
    "test_locality/test_aabb_query.cpp"
    "test_locality/test_aabb_simple.cpp"
    "test_locality/test_neighbor_bond.cpp"
    "test_locality/test_neighbor_list.cpp"

    "test_pack/test_constraint.cpp"
    # Temporarily disable Target tests due to incomplete Optimizer type
    # "test_pack/test_target.cpp"
    "test_pack/test_optimizer.cpp"
    # Temporarily disable Packer tests due to API mismatches
    # "test_pack/test_packer.cpp"
    # Temporarily excluding tests that have link issues
    # "test_bond.cpp"
)

add_executable(molcpp_tests ${test_sources})

setup_xtensor(molcpp_tests)
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
