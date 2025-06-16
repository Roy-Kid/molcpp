cmake_minimum_required(VERSION 3.21)

message(STATUS "Configuring molcpp tests")

# Create test executable
file(GLOB test_sources "test_*.cpp")
add_executable(molcpp_ecs_tests ${test_sources})

target_link_libraries(molcpp_ecs_tests PRIVATE 
    molcpp_objects
)

target_link_catch2(molcpp_ecs_tests)
target_link_xtensor(molcpp_ecs_tests)

target_include_directories(molcpp_ecs_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Register tests with CTest
catch_discover_tests(molcpp_ecs_tests)
