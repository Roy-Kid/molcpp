// molecule.cpp
// Python bindings for molcpp molecule classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <molcpp/molecule.hpp>

namespace py = pybind11;

void bind_molecule(py::module_& m) {
    // Molecule bindings module
    py::module_ molecule_module = m.def_submodule("molecule", "Molecule-related classes");
    
    // Molecule class
    py::class_<molcpp::Molecule>(molecule_module, "Molecule")
        .def(py::init<>(), "Create a new Molecule entity")
        .def("get_id", &molcpp::Molecule::get_id, "Get the unique ID of this molecule")
        .def("__repr__", [](const molcpp::Molecule& molecule) {
            return "<molcpp.molecule.Molecule id=" + std::to_string(molecule.get_id()) + ">";
        });
}
