# cmake/deps.cmake
# Centralized dependency management for molcpp

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

function(setup_catch2_target target_name)
    if(TARGET ${target_name})
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /W4)
        else()
            target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()

function(find_or_fetch_xtl)
  include(FetchContent)
  
  if(NOT TARGET xtl)
    find_package(xtl 0.8 QUIET)
    if(NOT xtl_FOUND)
      message(STATUS "xtl not found, fetching...")
      FetchContent_Declare(
        xtl
        GIT_REPOSITORY https://github.com/xtensor-stack/xtl.git
        GIT_TAG        0.8.0
      )
      FetchContent_MakeAvailable(xtl)
    else()
      message(STATUS "xtl found via find_package")
    endif()
  endif()
endfunction()

function(find_or_fetch_xtensor)
  include(FetchContent)
  
  # Ensure xtl is available first
  find_or_fetch_xtl()

  if(NOT TARGET xtensor)
    find_package(xtensor 0.27 QUIET)
    if(NOT xtensor_FOUND)
      message(STATUS "xtensor not found, fetching...")
      FetchContent_Declare(
        xtensor
        GIT_REPOSITORY https://github.com/xtensor-stack/xtensor.git
        GIT_TAG        0.27.0
      )
      FetchContent_MakeAvailable(xtensor)
    else()
      message(STATUS "xtensor found via find_package")
    endif()
  endif()
endfunction()

function(find_or_fetch_xtensor_blas)
  include(FetchContent)
  
  # Ensure xtensor is available first
  find_or_fetch_xtensor()
  
  if(NOT TARGET xtensor-blas)
    find_package(xtensor-blas 0.23 QUIET)
    if(NOT xtensor-blas_FOUND)
      message(STATUS "xtensor-blas not found, fetching...")
      FetchContent_Declare(
        xtensor_blas
        GIT_REPOSITORY https://github.com/xtensor-stack/xtensor-blas.git
        GIT_TAG        0.23.0
      )
      FetchContent_MakeAvailable(xtensor_blas)
    else()
      message(STATUS "xtensor-blas found via find_package")
    endif()
  endif()
endfunction()

function(find_or_fetch_xsimd)
  include(FetchContent)
  
  if(NOT TARGET xsimd)
    find_package(xsimd 13.2.0 QUIET)
    if(NOT xsimd_FOUND)
      message(STATUS "xsimd not found, fetching...")
      FetchContent_Declare(
        xsimd
        GIT_REPOSITORY https://github.com/xtensor-stack/xsimd.git
        GIT_TAG        13.2.0
      )
      FetchContent_MakeAvailable(xsimd)
    else()
      message(STATUS "xsimd found via find_package")
    endif()
  endif()
endfunction()


function(find_or_fetch_xtensor_python)
    if(TARGET xtensor-python)
        message(STATUS "xtensor-python target already exists")
        return()
    endif()

    # Try to find xtensor-python first
    find_package(xtensor-python CONFIG QUIET)
    
    if(xtensor-python_FOUND)
        message(STATUS "Found xtensor-python: ${xtensor-python_VERSION}")
        return()
    endif()

    # Download xtensor-python if not found
    message(STATUS "xtensor-python not found, downloading...")
    FetchContent_Declare(
        xtensor-python
        GIT_REPOSITORY https://github.com/Roy-Kid/xtensor-python
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(xtensor-python)
    message(STATUS "xtensor-python downloaded and configured")
endfunction()

function(setup_python_xtensor_target target_name)
    if(TARGET ${target_name})
        
        # Ensure required dependencies are found
        find_package(Python REQUIRED COMPONENTS Interpreter Development NumPy)
        find_package(pybind11 REQUIRED)
        
        # Link xtensor and xtensor-python
        target_link_libraries(${target_name} PRIVATE 
            xtensor 
            xtensor-python
            pybind11::module 
            pybind11::lto 
            pybind11::windows_extras
        )
        
        # Set common compiler flags
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /W4)
        else()
            target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        
        # Add Python and NumPy include directories
        target_include_directories(${target_name} PRIVATE 
            ${Python_INCLUDE_DIRS}
            ${Python_NumPy_INCLUDE_DIRS}
        )
        
        # Set pybind11 extension properties
        pybind11_extension(${target_name})
        if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
            pybind11_strip(${target_name})
        endif()
        
        set_target_properties(${target_name} PROPERTIES CXX_VISIBILITY_PRESET "hidden")
        
    else()
        message(FATAL_ERROR "Target ${target_name} not found")
    endif()
endfunction()
