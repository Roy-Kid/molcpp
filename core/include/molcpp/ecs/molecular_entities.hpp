#pragma once

/**
 * @file molecular_entities.hpp
 * @brief Example molecular entities that inherit from Entity.
 * 
 * This file demonstrates how to create domain-specific entities
 * for molecular modeling using the ECS framework.
 */

#include "molcpp/ecs/ecs.hpp"

namespace molcpp::molecular {

/**
 * @brief Atom entity that automatically has Position, Element, and Radius components.
 */
class Atom : public ecs::Entity {
public:
    /**
     * @brief Construct an Atom with element and position.
     * @param symbol Chemical symbol (e.g., "H", "C", "O").
     * @param atomic_number Atomic number.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @param radius Atomic radius (optional, defaults to 1.0).
     */
    Atom(const std::string& symbol, int atomic_number, 
         double x = 0.0, double y = 0.0, double z = 0.0, 
         double radius = 1.0) {
        add_component<ecs::components::Element>(symbol, atomic_number);
        add_component<ecs::components::Position>(x, y, z);
        add_component<ecs::components::Radius>(radius);
    }

    /**
     * @brief Get the element component.
     * @return const ecs::components::Element* Pointer to the element component.
     */
    const ecs::components::Element* element() const {
        return get_component<ecs::components::Element>();
    }

    /**
     * @brief Get the position component.
     * @return ecs::components::Position* Pointer to the position component.
     */
    ecs::components::Position* position() {
        return get_component<ecs::components::Position>();
    }

    /**
     * @brief Get the position component (const version).
     * @return const ecs::components::Position* Pointer to the position component.
     */
    const ecs::components::Position* position() const {
        return get_component<ecs::components::Position>();
    }

    /**
     * @brief Get the radius component.
     * @return ecs::components::Radius* Pointer to the radius component.
     */
    ecs::components::Radius* radius() {
        return get_component<ecs::components::Radius>();
    }

    /**
     * @brief Get the radius component (const version).
     * @return const ecs::components::Radius* Pointer to the radius component.
     */
    const ecs::components::Radius* radius() const {
        return get_component<ecs::components::Radius>();
    }
};

/**
 * @brief Bond entity representing a connection between two atoms.
 */
class Bond : public ecs::Entity {
public:
    /**
     * @brief Bond component storing atom IDs and bond order.
     */
    struct BondInfo {
        ecs::EntityId atom1_id;
        ecs::EntityId atom2_id;
        int bond_order;
        
        BondInfo(ecs::EntityId id1, ecs::EntityId id2, int order = 1)
            : atom1_id(id1), atom2_id(id2), bond_order(order) {}
            
        bool operator==(const BondInfo& other) const {
            return atom1_id == other.atom1_id && 
                   atom2_id == other.atom2_id && 
                   bond_order == other.bond_order;
        }
    };

    /**
     * @brief Construct a Bond between two atoms.
     * @param atom1_id ID of the first atom.
     * @param atom2_id ID of the second atom.
     * @param bond_order Bond order (1 = single, 2 = double, 3 = triple).
     */
    Bond(ecs::EntityId atom1_id, ecs::EntityId atom2_id, int bond_order = 1) {
        add_component<BondInfo>(atom1_id, atom2_id, bond_order);
    }

    /**
     * @brief Get the bond information.
     * @return const BondInfo* Pointer to the bond info component.
     */
    const BondInfo* bond_info() const {
        return get_component<BondInfo>();
    }
};

/**
 * @brief Molecule entity representing a collection of atoms and bonds.
 */
class Molecule : public ecs::Entity {
public:
    /**
     * @brief Molecule component storing metadata.
     */
    struct MoleculeInfo {
        std::string name;
        std::string formula;
        double molecular_weight;
        
        MoleculeInfo(const std::string& n = "", const std::string& f = "", double mw = 0.0)
            : name(n), formula(f), molecular_weight(mw) {}
            
        bool operator==(const MoleculeInfo& other) const {
            return name == other.name && 
                   formula == other.formula && 
                   molecular_weight == other.molecular_weight;
        }
    };

    /**
     * @brief Construct a Molecule.
     * @param name Molecule name.
     * @param formula Chemical formula.
     * @param molecular_weight Molecular weight.
     */
    Molecule(const std::string& name = "", const std::string& formula = "", double molecular_weight = 0.0) {
        add_component<MoleculeInfo>(name, formula, molecular_weight);
    }

    /**
     * @brief Get the molecule information.
     * @return const MoleculeInfo* Pointer to the molecule info component.
     */
    const MoleculeInfo* molecule_info() const {
        return get_component<MoleculeInfo>();
    }

    /**
     * @brief Get mutable molecule information.
     * @return MoleculeInfo* Pointer to the molecule info component.
     */
    MoleculeInfo* molecule_info() {
        return get_component<MoleculeInfo>();
    }
};

} // namespace molcpp::molecular
