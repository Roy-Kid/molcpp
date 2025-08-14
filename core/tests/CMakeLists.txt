cmake_minimum_required(VERSION 3.21)

message(STATUS "Configuring molcpp tests")

# Use centralized Catch2 dependency management
find_or_fetch_catch2()

# Ensure xtensor is available for spatial tests
find_or_fetch_xtensor()

# Create test executable
file(GLOB test_sources "test_*.cpp")
add_executable(molcpp_ecs_tests ${test_sources})

target_link_libraries(molcpp_ecs_tests PRIVATE 
    molcpp_objects
    Catch2::Catch2WithMain
    xtensor
)

target_include_directories(molcpp_ecs_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Setup common Catch2 target properties
setup_catch2_target(molcpp_ecs_tests)

# Register tests with CTest
catch_discover_tests(molcpp_ecs_tests)
