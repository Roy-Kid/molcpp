#pragma once

/**
 * @file molecule.hpp
 * @brief Abstract Molecule entity for molecular modeling.
 */

#include "molcpp/ecs/entity.hpp"

namespace molcpp {

/**
 * @brief Abstract Molecule entity that inherits from Entity.
 * A molecule is a collection of atoms and bonds forming a chemical entity.
 * Specific molecular properties can be added via components.
 */
class Molecule : public molcpp::ecs::Entity {
public:
    /**
     * @brief Default constructor for Molecule entity.
     */
    Molecule() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Molecule() = default;

    /**
     * @brief Copy constructor.
     */
    Molecule(const Molecule&) = delete;

    /**
     * @brief Move constructor.
     */
    Molecule(Molecule&&) = default;

    /**
     * @brief Copy assignment operator.
     */
    Molecule& operator=(const Molecule&) = delete;

    /**
     * @brief Move assignment operator.
     */
    Molecule& operator=(Molecule&&) = default;
};

} // namespace molcpp