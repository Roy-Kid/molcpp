// bond.cpp
// Python bindings for molcpp bond classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <molcpp/bond.hpp>

namespace py = pybind11;

void bind_bond(py::module_& m) {
    // Bond bindings module
    py::module_ bond_module = m.def_submodule("bond", "Bond-related classes");
    
    // Bond class
    py::class_<molcpp::Bond>(bond_module, "Bond")
        .def(py::init<>(), "Create a new Bond entity")
        .def("get_id", &molcpp::Bond::get_id, "Get the unique ID of this bond")
        .def("__repr__", [](const molcpp::Bond& bond) {
            return "<molcpp.bond.Bond id=" + std::to_string(bond.get_id()) + ">";
        });
}
