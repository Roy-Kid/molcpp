// ecs.cpp
// Simplified Python bindings for molcpp ECS - only expose Entity and Components

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <molcpp/ecs/entity.hpp>
#include <molcpp/ecs/components.hpp>
#include <unordered_map>
#include <functional>

namespace py = pybind11;

// Component registry for dynamic component handling
class ComponentRegistry {
private:
    // Function types for component operations
    using AddComponentFunc = std::function<py::object(molcpp::ecs::Entity&, py::args)>;
    using GetComponentFunc = std::function<py::object(molcpp::ecs::Entity&)>;
    using HasComponentFunc = std::function<bool(const molcpp::ecs::Entity&)>;
    using RemoveComponentFunc = std::function<bool(molcpp::ecs::Entity&)>;
    
    std::unordered_map<std::string, AddComponentFunc> add_funcs;
    std::unordered_map<std::string, GetComponentFunc> get_funcs;
    std::unordered_map<std::string, HasComponentFunc> has_funcs;
    std::unordered_map<std::string, RemoveComponentFunc> remove_funcs;
    
public:
    ComponentRegistry() {
        register_component<molcpp::ecs::components::Position>("Position");
        register_component<molcpp::ecs::components::Element>("Element");
        register_component<molcpp::ecs::components::Mass>("Mass");
        register_component<molcpp::ecs::components::Radius>("Radius");
        register_component<molcpp::ecs::components::Velocity>("Velocity");
        register_component<molcpp::ecs::components::Charge>("Charge");
        register_component<molcpp::ecs::components::BondInfo>("BondInfo");
    }
    
    template<typename T>
    void register_component(const std::string& name) {
        add_funcs[name] = [](molcpp::ecs::Entity& entity, py::args args) -> py::object {
            if constexpr (std::is_same_v<T, molcpp::ecs::components::Position>) {
                if (py::len(args) == 3) {
                    auto& comp = entity.add_component<T>(
                        args[0].cast<double>(), args[1].cast<double>(), args[2].cast<double>());
                    return py::cast(&comp, py::return_value_policy::reference);
                }
            } else if constexpr (std::is_same_v<T, molcpp::ecs::components::Element>) {
                if (py::len(args) == 2) {
                    auto& comp = entity.add_component<T>(
                        args[0].cast<std::string>(), args[1].cast<int>());
                    return py::cast(&comp, py::return_value_policy::reference);
                }
            } else if constexpr (std::is_same_v<T, molcpp::ecs::components::Mass> ||
                                 std::is_same_v<T, molcpp::ecs::components::Radius> ||
                                 std::is_same_v<T, molcpp::ecs::components::Charge>) {
                if (py::len(args) == 1) {
                    auto& comp = entity.add_component<T>(args[0].cast<double>());
                    return py::cast(&comp, py::return_value_policy::reference);
                }
            } else if constexpr (std::is_same_v<T, molcpp::ecs::components::Velocity>) {
                if (py::len(args) == 3) {
                    auto& comp = entity.add_component<T>(
                        args[0].cast<double>(), args[1].cast<double>(), args[2].cast<double>());
                    return py::cast(&comp, py::return_value_policy::reference);
                }
            } else if constexpr (std::is_same_v<T, molcpp::ecs::components::BondInfo>) {
                if (py::len(args) == 2) {
                    auto& comp = entity.add_component<T>(
                        args[0].cast<size_t>(), args[1].cast<size_t>());
                    return py::cast(&comp, py::return_value_policy::reference);
                }
            }
            // Default constructor fallback
            auto& comp = entity.add_component<T>();
            return py::cast(&comp, py::return_value_policy::reference);
        };
        
        get_funcs[name] = [](molcpp::ecs::Entity& entity) -> py::object {
            auto* comp = entity.get_component<T>();
            return comp ? py::cast(comp, py::return_value_policy::reference) : py::none();
        };
        
        has_funcs[name] = [](const molcpp::ecs::Entity& entity) -> bool {
            return entity.has_component<T>();
        };
        
        remove_funcs[name] = [](molcpp::ecs::Entity& entity) -> bool {
            return entity.remove_component<T>();
        };
    }
    
    py::object add_component(molcpp::ecs::Entity& entity, const std::string& type_name, py::args args) {
        auto it = add_funcs.find(type_name);
        if (it != add_funcs.end()) {
            return it->second(entity, args);
        }
        throw std::runtime_error("Unknown component type: " + type_name);
    }
    
    py::object get_component(molcpp::ecs::Entity& entity, const std::string& type_name) {
        auto it = get_funcs.find(type_name);
        if (it != get_funcs.end()) {
            return it->second(entity);
        }
        throw std::runtime_error("Unknown component type: " + type_name);
    }
    
    bool has_component(const molcpp::ecs::Entity& entity, const std::string& type_name) {
        auto it = has_funcs.find(type_name);
        if (it != has_funcs.end()) {
            return it->second(entity);
        }
        throw std::runtime_error("Unknown component type: " + type_name);
    }
    
    bool remove_component(molcpp::ecs::Entity& entity, const std::string& type_name) {
        auto it = remove_funcs.find(type_name);
        if (it != remove_funcs.end()) {
            return it->second(entity);
        }
        throw std::runtime_error("Unknown component type: " + type_name);
    }
};

// Global component registry instance
static ComponentRegistry component_registry;

void bind_ecs(py::module_& m) {
    // ECS bindings module - minimal exposure
    py::module_ ecs_module = m.def_submodule("ecs", "Entity Component System");
    
    // Entity class with dynamic component management
    py::class_<molcpp::ecs::Entity>(ecs_module, "Entity")
        .def(py::init<>(), "Create a new Entity")
        .def("get_id", &molcpp::ecs::Entity::get_id, "Get the unique ID of this entity")
        
        // Generic component management using the registry - pass component TYPE (class), not instance
        .def("add_component", [](molcpp::ecs::Entity& entity, py::object component_type, py::args args) -> py::object {
            std::string type_name = py::str(component_type.attr("__name__"));
            return component_registry.add_component(entity, type_name, args);
        }, "Add a component to this entity", py::arg("component_type"))
        
        .def("get_component", [](molcpp::ecs::Entity& entity, py::object component_type) -> py::object {
            std::string type_name = py::str(component_type.attr("__name__"));
            return component_registry.get_component(entity, type_name);
        }, "Get a component from this entity", py::arg("component_type"))
        
        .def("has_component", [](const molcpp::ecs::Entity& entity, py::object component_type) -> bool {
            std::string type_name = py::str(component_type.attr("__name__"));
            return component_registry.has_component(entity, type_name);
        }, "Check if entity has a component", py::arg("component_type"))
        
        .def("remove_component", [](molcpp::ecs::Entity& entity, py::object component_type) -> bool {
            std::string type_name = py::str(component_type.attr("__name__"));
            return component_registry.remove_component(entity, type_name);
        }, "Remove a component from this entity", py::arg("component_type"))
        
        .def("__repr__", [](const molcpp::ecs::Entity& entity) {
            return "<molcpp.ecs.Entity id=" + std::to_string(entity.get_id()) + ">";
        });
    
    // Component classes - only as data containers, no complex logic
    py::class_<molcpp::ecs::components::Position>(ecs_module, "Position")
        .def(py::init<>(), "Create position at origin")
        .def(py::init<double, double, double>(), "Create position with coordinates",
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def_readwrite("x", &molcpp::ecs::components::Position::x)
        .def_readwrite("y", &molcpp::ecs::components::Position::y)
        .def_readwrite("z", &molcpp::ecs::components::Position::z)
        .def("get_type_name", &molcpp::ecs::components::Position::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Position& pos) {
            return "<Position(" + std::to_string(pos.x) + ", " + 
                   std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")>";
        });
    
    py::class_<molcpp::ecs::components::Element>(ecs_module, "Element")
        .def(py::init<>(), "Create empty element")
        .def(py::init<const std::string&, int>(), "Create element",
             py::arg("symbol"), py::arg("atomic_number"))
        .def_readwrite("symbol", &molcpp::ecs::components::Element::symbol)
        .def_readwrite("atomic_number", &molcpp::ecs::components::Element::atomic_number)
        .def("get_type_name", &molcpp::ecs::components::Element::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Element& elem) {
            return "<Element " + elem.symbol + " (" + std::to_string(elem.atomic_number) + ")>";
        });
    
    py::class_<molcpp::ecs::components::Mass>(ecs_module, "Mass")
        .def(py::init<>(), "Create mass with value 0")
        .def(py::init<double>(), "Create mass", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Mass::value)
        .def("get_type_name", &molcpp::ecs::components::Mass::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Mass& mass) {
            return "<Mass " + std::to_string(mass.value) + ">";
        });
    
    py::class_<molcpp::ecs::components::Radius>(ecs_module, "Radius")
        .def(py::init<>(), "Create radius with value 0")
        .def(py::init<double>(), "Create radius", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Radius::value)
        .def("get_type_name", &molcpp::ecs::components::Radius::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Radius& radius) {
            return "<Radius " + std::to_string(radius.value) + ">";
        });
    
    py::class_<molcpp::ecs::components::Velocity>(ecs_module, "Velocity")
        .def(py::init<>(), "Create velocity at rest")
        .def(py::init<double, double, double>(), "Create velocity",
             py::arg("vx"), py::arg("vy"), py::arg("vz"))
        .def_readwrite("vx", &molcpp::ecs::components::Velocity::vx)
        .def_readwrite("vy", &molcpp::ecs::components::Velocity::vy)
        .def_readwrite("vz", &molcpp::ecs::components::Velocity::vz)
        .def("get_type_name", &molcpp::ecs::components::Velocity::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Velocity& vel) {
            return "<Velocity(" + std::to_string(vel.vx) + ", " + 
                   std::to_string(vel.vy) + ", " + std::to_string(vel.vz) + ")>";
        });
    
    py::class_<molcpp::ecs::components::Charge>(ecs_module, "Charge")
        .def(py::init<>(), "Create charge with value 0")
        .def(py::init<double>(), "Create charge", py::arg("value"))
        .def_readwrite("value", &molcpp::ecs::components::Charge::value)
        .def("get_type_name", &molcpp::ecs::components::Charge::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::Charge& charge) {
            return "<Charge " + std::to_string(charge.value) + ">";
        });
    
    py::class_<molcpp::ecs::components::BondInfo>(ecs_module, "BondInfo")
        .def(py::init<>(), "Create empty bond info")
        .def(py::init<size_t, size_t>(), "Create bond info",
             py::arg("atom1_id"), py::arg("atom2_id"))
        .def_readwrite("atom1_id", &molcpp::ecs::components::BondInfo::atom1_id)
        .def_readwrite("atom2_id", &molcpp::ecs::components::BondInfo::atom2_id)
        .def("get_type_name", &molcpp::ecs::components::BondInfo::get_type_name)
        .def("__repr__", [](const molcpp::ecs::components::BondInfo& info) {
            return "<BondInfo atoms=(" + std::to_string(info.atom1_id) + "," + 
                   std::to_string(info.atom2_id) + ")>";
        });
}
