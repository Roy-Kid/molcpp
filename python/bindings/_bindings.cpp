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
    
    // Bind all modules
    bind_atom(m);
    // bind_bond(m);
    // bind_molecule(m);
    // bind_spatial(m);
    // bind_ecs(m);
    // bind_utils(m);
}
