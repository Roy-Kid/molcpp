// ecs.cpp
// Simplified Python bindings for molcpp ECS module

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "molcpp/ecs/ecs.hpp"
#include "molcpp/ecs/entity.hpp"
#include "molcpp/ecs/components.hpp"

namespace py = pybind11;

void bind_ecs(py::module_& m) {
    using namespace molcpp::ecs;
    using namespace molcpp::ecs::components;
    
    py::module_ ecs = m.def_submodule("ecs", "Entity-Component-System framework");
    
    // Bind EntityId type
    py::class_<EntityId>(ecs, "EntityId")
        .def(py::init<size_t>())
        .def("__int__", [](const EntityId& id) { return static_cast<size_t>(id); })
        .def("__str__", [](const EntityId& id) { return std::to_string(static_cast<size_t>(id)); });
    
    // Bind Component base class
    py::class_<Component, std::shared_ptr<Component>>(ecs, "Component")
        .def("get_type_name", &Component::get_type_name)
        .def("get_type", &Component::get_type);
    
    // Bind Position component
    py::class_<Position, Component, std::shared_ptr<Position>>(ecs, "Position")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_readwrite("x", &Position::x)
        .def_readwrite("y", &Position::y)
        .def_readwrite("z", &Position::z)
        .def("__eq__", &Position::operator==)
        .def("__repr__", [](const Position& p) {
            return "Position(" + std::to_string(p.x) + ", " + 
                   std::to_string(p.y) + ", " + std::to_string(p.z) + ")";
        });
    
    // Bind Element component
    py::class_<Element, Component, std::shared_ptr<Element>>(ecs, "Element")
        .def(py::init<>())
        .def(py::init<const std::string&, int>())
        .def_readwrite("symbol", &Element::symbol)
        .def_readwrite("atomic_number", &Element::atomic_number)
        .def("__eq__", &Element::operator==)
        .def("__repr__", [](const Element& e) {
            return "Element('" + e.symbol + "', " + std::to_string(e.atomic_number) + ")";
        });
    
    // Bind Radius component
    py::class_<Radius, Component, std::shared_ptr<Radius>>(ecs, "Radius")
        .def(py::init<>())
        .def(py::init<double>())
        .def_readwrite("value", &Radius::value)
        .def("__eq__", &Radius::operator==)
        .def("__repr__", [](const Radius& r) {
            return "Radius(" + std::to_string(r.value) + ")";
        });
    
    // Bind Velocity component
    py::class_<Velocity, Component, std::shared_ptr<Velocity>>(ecs, "Velocity")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_readwrite("vx", &Velocity::vx)
        .def_readwrite("vy", &Velocity::vy)
        .def_readwrite("vz", &Velocity::vz)
        .def("__eq__", &Velocity::operator==)
        .def("__repr__", [](const Velocity& v) {
            return "Velocity(" + std::to_string(v.vx) + ", " + 
                   std::to_string(v.vy) + ", " + std::to_string(v.vz) + ")";
        });
    
    // Bind Mass component
    py::class_<Mass, Component, std::shared_ptr<Mass>>(ecs, "Mass")
        .def(py::init<>())
        .def(py::init<double>())
        .def_readwrite("value", &Mass::value)
        .def("__eq__", &Mass::operator==)
        .def("__repr__", [](const Mass& m) {
            return "Mass(" + std::to_string(m.value) + ")";
        });
    
    // Bind Charge component
    py::class_<Charge, Component, std::shared_ptr<Charge>>(ecs, "Charge")
        .def(py::init<>())
        .def(py::init<double>())
        .def_readwrite("value", &Charge::value)
        .def("__eq__", &Charge::operator==)
        .def("__repr__", [](const Charge& c) {
            return "Charge(" + std::to_string(c.value) + ")";
        });
    
    // Bind BondInfo component
    py::class_<BondInfo, Component, std::shared_ptr<BondInfo>>(ecs, "BondInfo")
        .def(py::init<>())
        .def(py::init<size_t, size_t>())
        .def_readwrite("atom1_id", &BondInfo::atom1_id)
        .def_readwrite("atom2_id", &BondInfo::atom2_id)
        .def("__eq__", &BondInfo::operator==)
        .def("__repr__", [](const BondInfo& b) {
            return "BondInfo(" + std::to_string(b.atom1_id) + ", " + std::to_string(b.atom2_id) + ")";
        });
    
    // Bind Entity class with simplified component management
    py::class_<Entity, std::shared_ptr<Entity>>(ecs, "Entity")
        .def(py::init<>())
        .def("get_id", &Entity::get_id)
        .def("add_position", [](Entity& e, double x, double y, double z) -> Position& {
            return e.add_component<Position>(x, y, z);
        })
        .def("add_element", [](Entity& e, const std::string& symbol, int atomic_number) -> Element& {
            return e.add_component<Element>(symbol, atomic_number);
        })
        .def("add_radius", [](Entity& e, double value) -> Radius& {
            return e.add_component<Radius>(value);
        })
        .def("add_velocity", [](Entity& e, double vx, double vy, double vz) -> Velocity& {
            return e.add_component<Velocity>(vx, vy, vz);
        })
        .def("add_mass", [](Entity& e, double value) -> Mass& {
            return e.add_component<Mass>(value);
        })
        .def("add_charge", [](Entity& e, double value) -> Charge& {
            return e.add_component<Charge>(value);
        })
        .def("add_bond_info", [](Entity& e, size_t atom1_id, size_t atom2_id) -> BondInfo& {
            return e.add_component<BondInfo>(atom1_id, atom2_id);
        })
        .def("get_position", [](Entity& e) -> Position* { return e.get_component<Position>(); })
        .def("get_element", [](Entity& e) -> Element* { return e.get_component<Element>(); })
        .def("get_radius", [](Entity& e) -> Radius* { return e.get_component<Radius>(); })
        .def("get_velocity", [](Entity& e) -> Velocity* { return e.get_component<Velocity>(); })
        .def("get_mass", [](Entity& e) -> Mass* { return e.get_component<Mass>(); })
        .def("get_charge", [](Entity& e) -> Charge* { return e.get_component<Charge>(); })
        .def("get_bond_info", [](Entity& e) -> BondInfo* { return e.get_component<BondInfo>(); })
        .def("has_position", [](Entity& e) -> bool { return e.has_component<Position>(); })
        .def("has_element", [](Entity& e) -> bool { return e.has_component<Element>(); })
        .def("has_radius", [](Entity& e) -> bool { return e.has_component<Radius>(); })
        .def("has_velocity", [](Entity& e) -> bool { return e.has_component<Velocity>(); })
        .def("has_mass", [](Entity& e) -> bool { return e.has_component<Mass>(); })
        .def("has_charge", [](Entity& e) -> bool { return e.has_component<Charge>(); })
        .def("has_bond_info", [](Entity& e) -> bool { return e.has_component<BondInfo>(); })
        .def("remove_position", [](Entity& e) -> bool { return e.remove_component<Position>(); })
        .def("remove_element", [](Entity& e) -> bool { return e.remove_component<Element>(); })
        .def("remove_radius", [](Entity& e) -> bool { return e.remove_component<Radius>(); })
        .def("remove_velocity", [](Entity& e) -> bool { return e.remove_component<Velocity>(); })
        .def("remove_mass", [](Entity& e) -> bool { return e.remove_component<Mass>(); })
        .def("remove_charge", [](Entity& e) -> bool { return e.remove_component<Charge>(); })
        .def("remove_bond_info", [](Entity& e) -> bool { return e.remove_component<BondInfo>(); });
}
