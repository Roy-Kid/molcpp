// ecs.cpp
// Python bindings for molcpp ECS (Entity Component System)

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pyarray.hpp>

#include <molcpp/ecs/ecs.hpp>
#include <molcpp/ecs/entity.hpp>
#include <molcpp/ecs/components.hpp>
#include <molcpp/ecs/system.hpp>
#include <molcpp/ecs/molecular_entities.hpp>
#include <molcpp/types.hpp>

namespace py = pybind11;

void bind_ecs(py::module_& m) {
    // ECS bindings module
    py::module_ ecs_module = m.def_submodule("ecs", "Entity Component System");
    
    // Entity class
    py::class_<molcpp::ecs::Entity>(ecs_module, "Entity")
        .def(py::init<>(), "Create a new Entity")
        .def("get_id", &molcpp::ecs::Entity::get_id, "Get the unique ID of this entity")
        .def("__repr__", [](const molcpp::ecs::Entity& entity) {
            return "<molcpp.ecs.Entity id=" + std::to_string(entity.get_id()) + ">";
        });
    
    // Components submodule
    py::module_ components = ecs_module.def_submodule("components", "ECS Components");
    
    // Position component with numpy array support
    py::class_<molcpp::ecs::components::Position>(components, "Position")
        .def(py::init<>(), "Create a position at origin")
        .def(py::init<double, double, double>(), "Create a position with x, y, z coordinates",
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def(py::init([](const xt::pyarray<double>& arr) {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements for Position");
            }
            return molcpp::ecs::components::Position(arr.flat(0), arr.flat(1), arr.flat(2));
        }), "Create position from numpy array", py::arg("array"))
        .def_readwrite("x", &molcpp::ecs::components::Position::x, "X coordinate")
        .def_readwrite("y", &molcpp::ecs::components::Position::y, "Y coordinate")
        .def_readwrite("z", &molcpp::ecs::components::Position::z, "Z coordinate")
        .def("to_array", [](const molcpp::ecs::components::Position& pos) {
            xt::pyarray<double> arr = xt::zeros<double>({3});
            arr(0) = pos.x;
            arr(1) = pos.y;
            arr(2) = pos.z;
            return arr;
        }, "Convert position to numpy array")
        .def("from_array", [](molcpp::ecs::components::Position& pos, const xt::pyarray<double>& arr) {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements");
            }
            pos.x = arr.flat(0);
            pos.y = arr.flat(1);
            pos.z = arr.flat(2);
        }, "Set position from numpy array", py::arg("array"))
        .def("__repr__", [](const molcpp::ecs::components::Position& pos) {
            return "<Position(" + std::to_string(pos.x) + ", " + 
                   std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")>";
        });
    
    // Element component
    py::class_<molcpp::ecs::components::Element>(components, "Element")
        .def(py::init<>(), "Create empty element")
        .def(py::init<const std::string&, int>(), "Create element with symbol and atomic number",
             py::arg("symbol"), py::arg("atomic_number"))
        .def_readwrite("symbol", &molcpp::ecs::components::Element::symbol, "Chemical symbol")
        .def_readwrite("atomic_number", &molcpp::ecs::components::Element::atomic_number, "Atomic number")
        .def("__repr__", [](const molcpp::ecs::components::Element& elem) {
            return "<Element " + elem.symbol + " (" + std::to_string(elem.atomic_number) + ")>";
        });
    
    // Radius component
    py::class_<molcpp::ecs::components::Radius>(components, "Radius")
        .def(py::init<>(), "Create radius with value 0")
        .def(py::init<double>(), "Create radius with given value", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Radius::value, "Radius value")
        .def("__repr__", [](const molcpp::ecs::components::Radius& radius) {
            return "<Radius " + std::to_string(radius.value) + ">";
        });
    
    // Velocity component with numpy array support
    py::class_<molcpp::ecs::components::Velocity>(components, "Velocity")
        .def(py::init<>(), "Create velocity at rest")
        .def(py::init<double, double, double>(), "Create velocity with vx, vy, vz components",
             py::arg("vx"), py::arg("vy"), py::arg("vz"))
        .def(py::init([](const xt::pyarray<double>& arr) {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements for Velocity");
            }
            return molcpp::ecs::components::Velocity(arr.flat(0), arr.flat(1), arr.flat(2));
        }), "Create velocity from numpy array", py::arg("array"))
        .def_readwrite("vx", &molcpp::ecs::components::Velocity::vx, "X velocity")
        .def_readwrite("vy", &molcpp::ecs::components::Velocity::vy, "Y velocity")
        .def_readwrite("vz", &molcpp::ecs::components::Velocity::vz, "Z velocity")
        .def("to_array", [](const molcpp::ecs::components::Velocity& vel) {
            xt::pyarray<double> arr = xt::zeros<double>({3});
            arr(0) = vel.vx;
            arr(1) = vel.vy;
            arr(2) = vel.vz;
            return arr;
        }, "Convert velocity to numpy array")
        .def("from_array", [](molcpp::ecs::components::Velocity& vel, const xt::pyarray<double>& arr) {
            if (arr.size() != 3) {
                throw std::invalid_argument("Array must have exactly 3 elements");
            }
            vel.vx = arr.flat(0);
            vel.vy = arr.flat(1);
            vel.vz = arr.flat(2);
        }, "Set velocity from numpy array", py::arg("array"))
        .def("__repr__", [](const molcpp::ecs::components::Velocity& vel) {
            return "<Velocity(" + std::to_string(vel.vx) + ", " + 
                   std::to_string(vel.vy) + ", " + std::to_string(vel.vz) + ")>";
        });
    
    // Mass component
    py::class_<molcpp::ecs::components::Mass>(components, "Mass")
        .def(py::init<>(), "Create mass with value 0")
        .def(py::init<double>(), "Create mass with given value", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Mass::value, "Mass value")
        .def("__repr__", [](const molcpp::ecs::components::Mass& mass) {
            return "<Mass " + std::to_string(mass.value) + ">";
        });
    
    // Charge component
    py::class_<molcpp::ecs::components::Charge>(components, "Charge")
        .def(py::init<>(), "Create charge with value 0")
        .def(py::init<double>(), "Create charge with given value", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Charge::value, "Charge value")
        .def("__repr__", [](const molcpp::ecs::components::Charge& charge) {
            return "<Charge " + std::to_string(charge.value) + ">";
        });
}
