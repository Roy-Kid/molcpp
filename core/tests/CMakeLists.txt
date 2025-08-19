cmake_minimum_required(VERSION 4.1)

message(STATUS "Configuring molcpp tests")

find_or_fetch_catch2()
find_or_fetch_xtensor()

# Create test executable with only spatial module tests
set(test_sources
    "test_spatial/test_box_boundary.cpp"
    "test_spatial/test_boundary.cpp"
    "test_spatial/test_regions.cpp"
    "test_locality/test_neighbor_bond.cpp"
    "test_locality/test_neighbor_list.cpp"
    "test_locality/test_aabb_simple.cpp"
    "test_locality/test_aabb_query.cpp"
    "test_locality/test_aabb_simd.cpp"
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
