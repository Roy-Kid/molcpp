#pragma once

/**
 * @file components.hpp
 * @brief Common component definitions for molecular modeling.
 * 
 * This file contains basic component structs that can be used with the ECS
 * framework for molecular modeling applications.
 */

#include <string>
#include <array>
#include <typeinfo>

namespace molcpp::ecs::components {

// Macro to generate standard component type methods
#define MOLCPP_COMPONENT_TYPE_METHODS(ComponentType) \
    const std::type_info& get_type() const override { \
        return typeid(ComponentType); \
    } \
    std::string get_type_name() const override { \
        return #ComponentType; \
    }

/**
 * @brief Base class for all components.
 * Provides type information for dynamic component handling.
 */
class Component {
public:
    virtual ~Component() = default;
    
    /**
     * @brief Get the type info of this component.
     * @return const std::type_info& Type information.
     */
    virtual const std::type_info& get_type() const = 0;
    
    /**
     * @brief Get a string representation of the component type.
     * @return std::string Component type name.
     */
    virtual std::string get_type_name() const = 0;
};

/**
 * @brief 3D position component.
 */
struct Position : public Component {
    double x, y, z;
    
    Position() : x(0.0), y(0.0), z(0.0) {}
    Position(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Position)
};

/**
 * @brief Chemical element component.
 */
struct Element : public Component {
    std::string symbol;
    int atomic_number;
    
    Element() : symbol(""), atomic_number(0) {}
    Element(const std::string& sym, int num) : symbol(sym), atomic_number(num) {}
    
    bool operator==(const Element& other) const {
        return symbol == other.symbol && atomic_number == other.atomic_number;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Element)
};

/**
 * @brief Atomic radius component.
 */
struct Radius : public Component {
    double value;
    
    Radius() : value(0.0) {}
    explicit Radius(double val) : value(val) {}
    
    bool operator==(const Radius& other) const {
        return value == other.value;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Radius)
};

/**
 * @brief Velocity component for molecular dynamics.
 */
struct Velocity : public Component {
    double vx, vy, vz;
    
    Velocity() : vx(0.0), vy(0.0), vz(0.0) {}
    Velocity(double vx_, double vy_, double vz_) : vx(vx_), vy(vy_), vz(vz_) {}
    
    bool operator==(const Velocity& other) const {
        return vx == other.vx && vy == other.vy && vz == other.vz;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Velocity)
};

/**
 * @brief Mass component.
 */
struct Mass : public Component {
    double value;
    
    Mass() : value(0.0) {}
    explicit Mass(double val) : value(val) {}
    
    bool operator==(const Mass& other) const {
        return value == other.value;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Mass)
};

/**
 * @brief Charge component for electrostatic calculations.
 */
struct Charge : public Component {
    double value;
    
    Charge() : value(0.0) {}
    explicit Charge(double val) : value(val) {}
    
    bool operator==(const Charge& other) const {
        return value == other.value;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(Charge)
};

/**
 * @brief Bond information component storing connection between atoms.
 */
struct BondInfo : public Component {
    size_t atom1_id;  ///< ID of the first atom
    size_t atom2_id;  ///< ID of the second atom
    
    BondInfo() : atom1_id(0), atom2_id(0) {}
    BondInfo(size_t id1, size_t id2) : atom1_id(id1), atom2_id(id2) {}
    
    bool operator==(const BondInfo& other) const {
        return atom1_id == other.atom1_id && atom2_id == other.atom2_id;
    }
    
    MOLCPP_COMPONENT_TYPE_METHODS(BondInfo)
};

} // namespace molcpp::ecs::components
