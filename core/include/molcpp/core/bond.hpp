#pragma once

/**
 * @file bond.hpp
 * @brief Abstract Bond entity for molecular modeling.
 */

#include "molcpp/core/ecs/entity.hpp"

namespace molcpp {

// Forward declaration
class Atom;

/**
 * @brief Abstract Bond entity representing a connection between entities.
 * Specific bond properties can be added via components.
 */
class Bond : public molcpp::ecs::Entity {
public:
    /**
     * @brief Default constructor for Bond entity.
     */
    Bond() = default;
    
    /**
     * @brief Constructor for Bond entity connecting two atoms.
     * @param atom1 Reference to the first atom.
     * @param atom2 Reference to the second atom.
     */
    Bond(const Atom& atom1, const Atom& atom2);

    /**
     * @brief Virtual destructor.
     */
    virtual ~Bond() = default;

    /**
     * @brief Copy constructor.
     */
    Bond(const Bond&) = delete;

    /**
     * @brief Move constructor.
     */
    Bond(Bond&&) = default;

    /**
     * @brief Copy assignment operator.
     */
    Bond& operator=(const Bond&) = delete;

    /**
     * @brief Move assignment operator.
     */
    Bond& operator=(Bond&&) = default;
};

} // namespace molcpp
