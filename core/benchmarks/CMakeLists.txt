cmake_minimum_required(VERSION 3.21)

# Benchmark executable
if(MOLCPP_BUILD_BENCHMARKS)
    message(STATUS "Building benchmarks")
    
    # Use centralized Catch2 dependency management
    find_or_fetch_catch2()
    
    # Ensure xtensor is available for benchmarks
    find_or_fetch_xtensor()

    # Collect all benchmark source files
    file(GLOB benchmark_sources "bench_*.cpp")
    
    if(benchmark_sources)
        add_executable(molcpp_benchmarks ${benchmark_sources})
        
        target_link_libraries(molcpp_benchmarks
            PRIVATE
                molcpp
                Catch2::Catch2WithMain
        )
        
        target_include_directories(molcpp_benchmarks
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/../include
        )
        
        # Setup common Catch2 target properties
        setup_catch2_target(molcpp_benchmarks)
        
        # Setup xtensor configuration for benchmarks
        setup_xtensor(molcpp_benchmarks)
        
        # Add to tests if CTest is enabled
        if(BUILD_TESTING)
            include(CTest)
            catch_discover_tests(molcpp_benchmarks)
        endif()
        
        message(STATUS "Benchmark executable: molcpp_benchmarks")
    else()
        message(STATUS "No benchmark source files found")
    endif()
else()
    message(STATUS "Benchmarks disabled (MOLCPP_BUILD_BENCHMARKS=OFF)")
endif()
