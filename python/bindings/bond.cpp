// bond.cpp
// Python bindings for molcpp bond classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <molcpp/bond.hpp>
#include <molcpp/atom.hpp>
#include <molcpp/ecs/components.hpp>

namespace py = pybind11;

void bind_bond(py::module_& m) {
    // Bond bindings module
    py::module_ bond_module = m.def_submodule("bond", "Bond-related classes");
    
    // Bond class - inherits from Entity
    py::class_<molcpp::Bond, molcpp::ecs::Entity>(bond_module, "Bond")
        .def(py::init<>(), "Create a new Bond entity")
        .def(py::init<const molcpp::Atom&, const molcpp::Atom&>(), 
             "Create a new Bond between two atoms",
             py::arg("atom1"), py::arg("atom2"))
        
        // Convenience methods
        .def("get_atoms", [](const molcpp::Bond& bond) -> py::tuple {
            auto* info = bond.get_component<molcpp::ecs::components::BondInfo>();
            if (info) {
                return py::make_tuple(info->atom1_id, info->atom2_id);
            }
            return py::make_tuple(0, 0);
        }, "Get the IDs of the two atoms connected by this bond")
        
        .def("__repr__", [](const molcpp::Bond& bond) {
            auto* info = bond.get_component<molcpp::ecs::components::BondInfo>();
            if (info) {
                return "<molcpp.bond.Bond id=" + std::to_string(bond.get_id()) + 
                       " atoms=(" + std::to_string(info->atom1_id) + "," + std::to_string(info->atom2_id) + ")>";
            }
            return "<molcpp.bond.Bond id=" + std::to_string(bond.get_id()) + ">";
        });
}
