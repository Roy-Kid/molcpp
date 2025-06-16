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

namespace molcpp::ecs::components {

/**
 * @brief 3D position component.
 */
struct Position {
    double x, y, z;
    
    Position() : x(0.0), y(0.0), z(0.0) {}
    Position(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

/**
 * @brief Chemical element component.
 */
struct Element {
    std::string symbol;
    int atomic_number;
    
    Element() : symbol(""), atomic_number(0) {}
    Element(const std::string& sym, int num) : symbol(sym), atomic_number(num) {}
    
    bool operator==(const Element& other) const {
        return symbol == other.symbol && atomic_number == other.atomic_number;
    }
};

/**
 * @brief Atomic radius component.
 */
struct Radius {
    double value;
    
    Radius() : value(0.0) {}
    explicit Radius(double val) : value(val) {}
    
    bool operator==(const Radius& other) const {
        return value == other.value;
    }
};

/**
 * @brief Velocity component for molecular dynamics.
 */
struct Velocity {
    double vx, vy, vz;
    
    Velocity() : vx(0.0), vy(0.0), vz(0.0) {}
    Velocity(double vx_, double vy_, double vz_) : vx(vx_), vy(vy_), vz(vz_) {}
    
    bool operator==(const Velocity& other) const {
        return vx == other.vx && vy == other.vy && vz == other.vz;
    }
};

/**
 * @brief Mass component.
 */
struct Mass {
    double value;
    
    Mass() : value(0.0) {}
    explicit Mass(double val) : value(val) {}
    
    bool operator==(const Mass& other) const {
        return value == other.value;
    }
};

/**
 * @brief Charge component for electrostatic calculations.
 */
struct Charge {
    double value;
    
    Charge() : value(0.0) {}
    explicit Charge(double val) : value(val) {}
    
    bool operator==(const Charge& other) const {
        return value == other.value;
    }
};

} // namespace molcpp::ecs::components
