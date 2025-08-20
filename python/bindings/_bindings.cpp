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
    
    // Bind modules in dependency order
    bind_ecs(m);      // Entity-Component-System framework
    bind_spatial(m);  // Spatial geometry and boundaries
    bind_locality(m); // Locality and neighbor finding
}
