# cmake/deps.cmake
# Centralized dependency management for molcpp

# Function to find or fetch Catch2
function(find_or_fetch_catch2)
    # Only fetch if not already available
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
        target_compile_features(${target_name} PRIVATE cxx_std_20)
        
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
            
            FetchContent_Declare(
                xtl
                GIT_REPOSITORY https://github.com/xtensor-stack/xtl.git
                GIT_TAG        0.8.0
            )
            FetchContent_MakeAvailable(xtl)
            
            FetchContent_Declare(
                xtensor
                GIT_REPOSITORY https://github.com/xtensor-stack/xtensor.git
                GIT_TAG        0.27.0
            )
            FetchContent_MakeAvailable(xtensor)

            FetchContent_Declare(
                xtensor-blas
                GIT_REPOSITORY https://github.com/xtensor-stack/xtensor-blas.git
                GIT_TAG        master
            )
            FetchContent_MakeAvailable(xtensor-blas)
        endif()
    else()
        message(STATUS "xtensor already available")
    endif()
endfunction()

function(setup_xtensor target_name)

    if(TARGET ${target_name})
        target_compile_features(${target_name} PRIVATE cxx_std_20)
        
        # Set common compiler flags for xtensor targets
        if(MSVC)
            set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
        endif()

        # find_package(xtensor REQUIRED)
        # target_include_directories(${target_name} PUBLIC ${xtensor_INCLUDE_DIRS})
        target_link_libraries(${target_name} PUBLIC xtensor xtensor-blas)

        # xtensor-blas
        add_definitions(-DHAVE_CBLAS=1)
        if (WIN32)
            find_package(OpenBLAS REQUIRED)
            set(BLAS_LIBRARIES ${CMAKE_INSTALL_PREFIX}${OpenBLAS_LIBRARIES})
        else()
            find_package(BLAS REQUIRED)
            find_package(LAPACK REQUIRED)
        endif()
        message(STATUS "BLAS VENDOR:    " ${BLA_VENDOR})
        message(STATUS "BLAS LIBRARIES: " ${BLAS_LIBRARIES})
        target_link_libraries(${target_name} PUBLIC ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})

    else()
        message(FATAL_ERROR "Target ${target_name} not found")
    endif()
endfunction()