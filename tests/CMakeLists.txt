message(STATUS "Building tests")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    molcpp_test
    test_graph.cpp
)

target_link_libraries(
    molcpp_test
    GTest::gtest_main
    molcpp
)

target_compile_options(molcpp_test
    PRIVATE
        -O1
        -Wall
        -g
)

include(GoogleTest)
gtest_discover_tests(molcpp_test)