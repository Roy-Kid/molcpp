// bindings.hpp
// Header file for molcpp Python bindings

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Function declarations for each binding module
void bind_spatial(py::module_& m);
void bind_locality(py::module_& m);
void bind_ecs(py::module_& m);
