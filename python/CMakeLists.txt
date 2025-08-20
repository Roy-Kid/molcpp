# Python bindings CMakeLists.txt

# Find Python and components first
find_package(Python REQUIRED COMPONENTS Interpreter Development NumPy)
message(STATUS "Found Python: ${Python_EXECUTABLE}")
message(STATUS "Python version: ${Python_VERSION}")
message(STATUS "Python NumPy: ${Python_NumPy_INCLUDE_DIRS}")

# Find pybind11 after Python (official xtensor-python order)
find_package(pybind11 REQUIRED CONFIG)
message(STATUS "Found pybind11: ${pybind11_VERSION}")

# Find xtensor-python (should be available from root deps)
# find_package(xtensor-python REQUIRED)
message(STATUS "Found xtensor-python: ${xtensor-python_VERSION}")

# Create Python extension module
pybind11_add_module(_bindings 
    bindings/_bindings.cpp
    bindings/ecs.cpp
    bindings/spatial.cpp
    bindings/locality.cpp
)

# Set module properties
set_target_properties(_bindings PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
    SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
    POSITION_INDEPENDENT_CODE TRUE
)

# Link libraries in the correct order (following xtensor-python examples)
target_link_libraries(_bindings PUBLIC 
    pybind11::module 
    xtensor-python 
    Python::NumPy
)

# Link molcpp core if available
if(TARGET molcpp)
    target_link_libraries(_bindings PRIVATE molcpp)
    message(STATUS "Linking Python bindings with molcpp core library")
endif()

setup_xtensor(_bindings)

# Set version info
target_compile_definitions(_bindings PRIVATE VERSION_INFO=${PROJECT_VERSION})

# Enable debug symbols in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(_bindings PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES)
endif()

message(STATUS "Python extension module '_bindings' configured successfully")
target_include_directories(_bindings PRIVATE 
    ${Python_INCLUDE_DIRS}
    ${Python_NumPy_INCLUDE_DIRS}
)

# Set pybind11 extension properties
# pybind11_extension(_bindings)
# if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
#     pybind11_strip(_bindings)
# endif()

# set_target_properties(_bindings PROPERTIES CXX_VISIBILITY_PRESET "hidden")

# Set output properties
set_target_properties(_bindings PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/molcpp
    OUTPUT_NAME "_bindings"
)
