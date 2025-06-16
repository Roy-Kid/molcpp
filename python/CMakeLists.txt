cmake_minimum_required(VERSION 3.16)

# Prevent CMake from looking for C/C++ compilers unnecessarily
project(molcpp-python NONE)

option(MOLCPP_PY_INTERNAL "Use the internal version of molcpp" OFF)

file(REMOVE ${PROJECT_SOURCE_DIR}/src/molcpp/external.py)

if(NOT ${MOLCPP_PY_INTERNAL})
    find_package(molcpp CONFIG QUIET)
endif()

if(${molcpp_FOUND})
    set(MOLCPP_VERSION "${molcpp_VERSION}")
    get_target_property(MOLCPP_TYPE molcpp TYPE)
    if (NOT "${MOLCPP_TYPE}" STREQUAL "SHARED_LIBRARY")
        message(FATAL_ERROR
            "We can only use a shared library build of molcpp.\n"
            "Found a ${MOLCPP_TYPE} build at ${MOLCPP_LOCATION}\n"
            "Define MOLCPP_PY_INTERNAL=ON to use the internal build instead."
        )
    endif()

    # Get the path to the chemfiles library. Instead of using the actual full
    # path to the library, we want to use the path to the symlink containing
    # only the SONAME/SOVERSION. For example, instead of the actual file
    # `/usr/local/lib/libchemfiles.0.11.3.dylib`, we want the symlink
    # `/usr/local/lib/libchemfiles.0.11.dylib`. This allows the same python
    # package to load different bugfix versions of chemfiles when loading an
    # external library.
    get_target_property(MOLCPP_FULL_LOCATION molcpp LOCATION)
    get_target_property(MOLCPP_SONAME molcpp IMPORTED_SONAME)

    if(NOT "${MOLCPP_SONAME}" STREQUAL "MOLCPP_SONAME-NOTFOUND")
        get_filename_component(MOLCPP_SOLINK ${MOLCPP_SONAME} NAME)
        get_filename_component(MOLCPP_DIR ${MOLCPP_FULL_LOCATION} DIRECTORY)
        set(MOLCPP_LOCATION ${MOLCPP_DIR}/${MOLCPP_SOLINK})
    else()
        set(MOLCPP_LOCATION ${MOLCPP_FULL_LOCATION})
    endif()

    if (EXISTS ${MOLCPP_LOCATION})
        message(STATUS "Using external molcpp ${MOLCPP_VERSION} at ${MOLCPP_LOCATION}")
    else()
        message(FATAL_ERROR "Missing symbolic link to ${MOLCPP_FULL_LOCATION}, expected at ${MOLCPP_LOCATION}")
    endif()

    file(WRITE ${PROJECT_SOURCE_DIR}/src/molcpp/_external.py
        "EXTERNAL_MOLCPP_PATH = \"${MOLCPP_LOCATION}\"\n"
    )

    install(CODE "message(STATUS \"nothing to install\")")

else()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/CMakeLists.txt")
        message(FATAL_ERROR
            "The git submodule for molcpp is not initialized.\n"
            "Please run `git submodule update --init`"
        )
    endif()

    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /LTCG")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /LTCG")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
    endif()

    message(STATUS "Using internal molcpp from ${CMAKE_CURRENT_SOURCE_DIR}/lib")

    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries instead of static ones" FORCE)

    add_subdirectory(lib EXCLUDE_FROM_ALL)

    add_custom_target(build_molcpp ALL)
    add_dependencies(build_molcpp molcpp)

    # Remove version info to avoid duplicated libmolcpp in Python wheels
    set_property(TARGET molcpp PROPERTY VERSION)
    set_property(TARGET molcpp PROPERTY SOVERSION)

    install(TARGETS molcpp
        LIBRARY DESTINATION "molcpp"
        RUNTIME DESTINATION "molcpp"
    )
endif()
