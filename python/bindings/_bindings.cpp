// _bindings.cpp
// Main Python bindings module for molcpp

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#define FORCE_IMPORT_ARRAY
#include <xtensor-python/pyarray.hpp>

namespace py = pybind11;

PYBIND11_MODULE(_bindings, m) {
    xt::import_numpy();
    
    m.doc() = "molcpp: Molecular modeling library with xtensor integration";
    
    // Bind modules in dependency order: ECS first (base classes), then derived classes
    bind_ecs(m);      // Must be first - contains Entity base class
    bind_atom(m);     // Depends on Entity
    bind_bond(m);     // Depends on Entity
    bind_spatial(m);  // Spatial geometry and boundaries
    // bind_utils(m);
}
