// atom.cpp
// Python bindings for molcpp atom classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pyarray.hpp>
#include <molcpp/atom.hpp>
#include <molcpp/ecs/components.hpp>

namespace py = pybind11;

void bind_atom(py::module_& m) {
    // Atom bindings - inherits from Entity
    py::module_ atom_module = m.def_submodule("atom", "Atom-related classes");
    
    py::class_<molcpp::Atom, molcpp::ecs::Entity>(atom_module, "Atom")
        .def(py::init<>(), "Create a new Atom entity")
        
        // Convenience property access methods
        .def("set_position", [](molcpp::Atom& atom, double x, double y, double z) {
            auto* pos = atom.get_component<molcpp::ecs::components::Position>();
            if (pos) {
                pos->x = x; pos->y = y; pos->z = z;
            } else {
                atom.add_component<molcpp::ecs::components::Position>(x, y, z);
            }
        }, "Set or update position", py::arg("x"), py::arg("y"), py::arg("z"))
        
        .def("get_position", [](const molcpp::Atom& atom) -> py::tuple {
            auto* pos = atom.get_component<molcpp::ecs::components::Position>();
            if (pos) {
                return py::make_tuple(pos->x, pos->y, pos->z);
            }
            return py::make_tuple(0.0, 0.0, 0.0);
        }, "Get position as (x, y, z) tuple")
        
        .def("set_element", [](molcpp::Atom& atom, const std::string& symbol, int atomic_number) {
            auto* elem = atom.get_component<molcpp::ecs::components::Element>();
            if (elem) {
                elem->symbol = symbol; elem->atomic_number = atomic_number;
            } else {
                atom.add_component<molcpp::ecs::components::Element>(symbol, atomic_number);
            }
        }, "Set or update element", py::arg("symbol"), py::arg("atomic_number"))
        
        .def("get_element", [](const molcpp::Atom& atom) -> py::tuple {
            auto* elem = atom.get_component<molcpp::ecs::components::Element>();
            if (elem) {
                return py::make_tuple(elem->symbol, elem->atomic_number);
            }
            return py::make_tuple("", 0);
        }, "Get element as (symbol, atomic_number) tuple")
        
        .def("set_mass", [](molcpp::Atom& atom, double mass) {
            auto* m = atom.get_component<molcpp::ecs::components::Mass>();
            if (m) {
                m->value = mass;
            } else {
                atom.add_component<molcpp::ecs::components::Mass>(mass);
            }
        }, "Set or update mass", py::arg("mass"))
        
        .def("get_mass", [](const molcpp::Atom& atom) -> double {
            auto* m = atom.get_component<molcpp::ecs::components::Mass>();
            return m ? m->value : 0.0;
        }, "Get mass value")
        
        .def("set_radius", [](molcpp::Atom& atom, double radius) {
            auto* r = atom.get_component<molcpp::ecs::components::Radius>();
            if (r) {
                r->value = radius;
            } else {
                atom.add_component<molcpp::ecs::components::Radius>(radius);
            }
        }, "Set or update radius", py::arg("radius"))
        
        .def("get_radius", [](const molcpp::Atom& atom) -> double {
            auto* r = atom.get_component<molcpp::ecs::components::Radius>();
            return r ? r->value : 0.0;
        }, "Get radius value")
        
        .def("__repr__", [](const molcpp::Atom& atom) {
            std::string result = "<molcpp.atom.Atom id=" + std::to_string(atom.get_id());
            
            auto* elem = atom.get_component<molcpp::ecs::components::Element>();
            if (elem) {
                result += " element=" + elem->symbol;
            }
            
            auto* pos = atom.get_component<molcpp::ecs::components::Position>();
            if (pos) {
                result += " pos=(" + std::to_string(pos->x) + "," + 
                         std::to_string(pos->y) + "," + std::to_string(pos->z) + ")";
            }
            
            result += ">";
            return result;
        });
}
