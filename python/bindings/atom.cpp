// atom.cpp
// Python bindings for molcpp atom classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pyarray.hpp>
#include <molcpp/atom.hpp>
#include <molcpp/ecs/components.hpp>
#include <molcpp/types.hpp>

namespace py = pybind11;

void bind_atom(py::module_& m) {
    // Atom bindings
    py::module_ atom_module = m.def_submodule("atom", "Atom-related classes");
    
    py::class_<molcpp::Atom>(atom_module, "Atom")
        .def(py::init<>(), "Create a new Atom entity")
        .def("get_id", &molcpp::Atom::get_id, "Get the unique ID of this atom")
        
        // Component management using lambdas to handle templates
        .def("add_position", [](molcpp::Atom& atom, double x, double y, double z) -> molcpp::ecs::components::Position& {
            return atom.add_component<molcpp::ecs::components::Position>(x, y, z);
        }, "Add a Position component", py::arg("x") = 0.0, py::arg("y") = 0.0, py::arg("z") = 0.0,
        py::return_value_policy::reference_internal)
        
        .def("add_position_from_array", [](molcpp::Atom& atom, const xt::pyarray<double>& arr) -> molcpp::ecs::components::Position& {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements for Position");
            }
            return atom.add_component<molcpp::ecs::components::Position>(arr.flat(0), arr.flat(1), arr.flat(2));
        }, "Add a Position component from numpy array", py::arg("array"),
        py::return_value_policy::reference_internal)
        
        .def("add_element", [](molcpp::Atom& atom, const std::string& symbol, int atomic_number) -> molcpp::ecs::components::Element& {
            return atom.add_component<molcpp::ecs::components::Element>(symbol, atomic_number);
        }, "Add an Element component", py::arg("symbol"), py::arg("atomic_number"),
        py::return_value_policy::reference_internal)
        
        .def("add_radius", [](molcpp::Atom& atom, double value) -> molcpp::ecs::components::Radius& {
            return atom.add_component<molcpp::ecs::components::Radius>(value);
        }, "Add a Radius component", py::arg("value"),
        py::return_value_policy::reference_internal)
        
        .def("add_velocity", [](molcpp::Atom& atom, double vx, double vy, double vz) -> molcpp::ecs::components::Velocity& {
            return atom.add_component<molcpp::ecs::components::Velocity>(vx, vy, vz);
        }, "Add a Velocity component", py::arg("vx") = 0.0, py::arg("vy") = 0.0, py::arg("vz") = 0.0,
        py::return_value_policy::reference_internal)
        
        .def("add_velocity_from_array", [](molcpp::Atom& atom, const xt::pyarray<double>& arr) -> molcpp::ecs::components::Velocity& {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements for Velocity");
            }
            return atom.add_component<molcpp::ecs::components::Velocity>(arr.flat(0), arr.flat(1), arr.flat(2));
        }, "Add a Velocity component from numpy array", py::arg("array"),
        py::return_value_policy::reference_internal)
        
        .def("add_mass", [](molcpp::Atom& atom, double value) -> molcpp::ecs::components::Mass& {
            return atom.add_component<molcpp::ecs::components::Mass>(value);
        }, "Add a Mass component", py::arg("value"),
        py::return_value_policy::reference_internal)
        
        .def("add_charge", [](molcpp::Atom& atom, double value) -> molcpp::ecs::components::Charge& {
            return atom.add_component<molcpp::ecs::components::Charge>(value);
        }, "Add a Charge component", py::arg("value"),
        py::return_value_policy::reference_internal)
        
        // Component getters
        .def("get_position", [](molcpp::Atom& atom) -> molcpp::ecs::components::Position* {
            return atom.get_component<molcpp::ecs::components::Position>();
        }, "Get Position component", py::return_value_policy::reference_internal)
        
        .def("get_element", [](molcpp::Atom& atom) -> molcpp::ecs::components::Element* {
            return atom.get_component<molcpp::ecs::components::Element>();
        }, "Get Element component", py::return_value_policy::reference_internal)
        
        .def("get_radius", [](molcpp::Atom& atom) -> molcpp::ecs::components::Radius* {
            return atom.get_component<molcpp::ecs::components::Radius>();
        }, "Get Radius component", py::return_value_policy::reference_internal)
        
        .def("get_velocity", [](molcpp::Atom& atom) -> molcpp::ecs::components::Velocity* {
            return atom.get_component<molcpp::ecs::components::Velocity>();
        }, "Get Velocity component", py::return_value_policy::reference_internal)
        
        .def("get_mass", [](molcpp::Atom& atom) -> molcpp::ecs::components::Mass* {
            return atom.get_component<molcpp::ecs::components::Mass>();
        }, "Get Mass component", py::return_value_policy::reference_internal)
        
        .def("get_charge", [](molcpp::Atom& atom) -> molcpp::ecs::components::Charge* {
            return atom.get_component<molcpp::ecs::components::Charge>();
        }, "Get Charge component", py::return_value_policy::reference_internal)
        
        // Component existence checks
        .def("has_position", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Position>();
        }, "Check if atom has Position component")
        
        .def("has_element", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Element>();
        }, "Check if atom has Element component")
        
        .def("has_radius", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Radius>();
        }, "Check if atom has Radius component")
        
        .def("has_velocity", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Velocity>();
        }, "Check if atom has Velocity component")
        
        .def("has_mass", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Mass>();
        }, "Check if atom has Mass component")
        
        .def("has_charge", [](const molcpp::Atom& atom) {
            return atom.has_component<molcpp::ecs::components::Charge>();
        }, "Check if atom has Charge component")
        
        // Component removal
        .def("remove_position", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Position>();
        }, "Remove Position component")
        
        .def("remove_element", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Element>();
        }, "Remove Element component")
        
        .def("remove_radius", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Radius>();
        }, "Remove Radius component")
        
        .def("remove_velocity", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Velocity>();
        }, "Remove Velocity component")
        
        .def("remove_mass", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Mass>();
        }, "Remove Mass component")
        
        .def("remove_charge", [](molcpp::Atom& atom) {
            return atom.remove_component<molcpp::ecs::components::Charge>();
        }, "Remove Charge component")
        
        .def("__repr__", [](const molcpp::Atom& atom) {
            return "<molcpp.atom.Atom id=" + std::to_string(atom.get_id()) + ">";
        });
}
