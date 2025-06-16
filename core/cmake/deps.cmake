# cmake/deps.cmake
# Centralized dependency management for molcpp

# Function to find or fetch Catch2
function(find_or_fetch_catch2)
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
endfunction()

# Function to setup common Catch2 target properties
function(setup_catch2_target target_name)
    if(TARGET ${target_name})
        target_compile_features(${target_name} PRIVATE cxx_std_17)
        
        # Set common compiler flags for Catch2 targets
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /W4)
        else()
            target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()

function(find_or_fetch_xtensor)
    if(NOT TARGET xtensor)
        find_package(xtensor 0.24.0 QUIET)

        if(NOT xtensor_FOUND)
            message(STATUS "xtensor not found, downloading...")
            include(FetchContent)
            
            # First fetch xtl (xtensor dependency)
            FetchContent_Declare(
                xtl
                GIT_REPOSITORY https://github.com/xtensor-stack/xtl.git
                GIT_TAG        0.8.0
            )
            FetchContent_MakeAvailable(xtl)
            
            # Then fetch xtensor
            FetchContent_Declare(
                xtensor
                GIT_REPOSITORY https://github.com/xtensor-stack/xtensor.git
                GIT_TAG        0.26.0
            )
            FetchContent_MakeAvailable(xtensor)
        endif()
    else()
        message(STATUS "xtensor already available")
    endif()
endfunction()

function(setup_xtensor target_name)
    if(TARGET ${target_name})
        target_compile_features(${target_name} PRIVATE cxx_std_17)
        
        # Set common compiler flags for xtensor targets
        if(MSVC)
            set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
        endif()
        
        # Link to xtensor target - this automatically handles include directories
        target_link_libraries(${target_name} PUBLIC xtensor)
        
        message(STATUS "Setup xtensor for ${target_name}")
    endif()
endfunction()