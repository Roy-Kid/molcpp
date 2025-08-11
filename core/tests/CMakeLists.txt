cmake_minimum_required(VERSION 3.21)

message(STATUS "Configuring molcpp tests")

# Create test executable for ECS tests
file(GLOB ecs_test_sources "test_ecs_*.cpp")
if(ecs_test_sources)
    add_executable(molcpp_ecs_tests ${ecs_test_sources})
    target_link_libraries(molcpp_ecs_tests PRIVATE molcpp_objects)
    target_link_catch2(molcpp_ecs_tests)
    target_link_xtensor(molcpp_ecs_tests)
    target_include_directories(molcpp_ecs_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    catch_discover_tests(molcpp_ecs_tests)
endif()

# Create test executable for locality tests
file(GLOB locality_test_sources "test_locality_*.cpp")
if(locality_test_sources)
    add_executable(molcpp_locality_tests ${locality_test_sources})
    target_link_libraries(molcpp_locality_tests PRIVATE molcpp_objects molcpp)
    target_link_catch2(molcpp_locality_tests)
    target_link_xtensor(molcpp_locality_tests)
    
    # Link OpenMP if available
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(molcpp_locality_tests PRIVATE OpenMP::OpenMP_CXX)
    endif()
    
    # Link xsimd if available
    find_package(xsimd)
    if(xsimd_FOUND)
        target_link_libraries(molcpp_locality_tests PRIVATE xsimd)
    endif()
    
    target_include_directories(molcpp_locality_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    catch_discover_tests(molcpp_locality_tests)
endif()
