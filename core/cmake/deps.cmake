# cmake/deps.cmake
# Centralized dependency management for molcpp

# Function to find or fetch Catch2
function(target_link_catch2 molcpp_target)
    if(NOT TARGET Catch2::Catch2WithMain)
        find_package(Catch2 3 QUIET)
        
        if(NOT Catch2_FOUND)
            message(STATUS "Catch2 not found, downloading...")
            include(FetchContent)
            FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG        v3.4.0
            )
            FetchContent_MakeAvailable(Catch2)
            
            # Add Catch module path for catch_discover_tests
            list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
        endif()
        
        # Include Catch module for catch_discover_tests function
        include(Catch)
    else()
        message(STATUS "Catch2 already available")
    endif()

    target_link_libraries(${molcpp_target} PRIVATE Catch2::Catch2WithMain)

endfunction()

# Function to find or fetch xtensor
function(target_link_xtensor molcpp_target)
    if(NOT TARGET xtensor)
        message(STATUS "Checking for xtensor...")
        find_package(xtensor 0.26.0 QUIET)
        if(NOT xtensor_FOUND)
            message(ERROR "NOT SUPPORT fetch content yet")
        elseif()
            message(STATUS "Found xtensor version ${xtensor_VERSION}")           
        endif()
    else()
        message(STATUS "xtensor already available")
    endif()
    target_include_directories(${molcpp_target} PUBLIC ${xtensor_INCLUDE_DIRS})
    target_link_libraries(${molcpp_target} PUBLIC xtensor)
endfunction()