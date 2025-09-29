#pragma once

/**
 * @file angle.hpp
 * @brief Abstract Angle entity for molecular modeling.
 */

#include "molcpp/core/ecs/entity.hpp"

namespace molcpp {

// Forward declarations
class Atom;

/**
 * @brief Abstract Angle entity representing an angle between three atoms.
 * Specific angle properties can be added via components.
 */
class Angle : public molcpp::ecs::Entity {
public:
    /**
     * @brief Default constructor for Angle entity.
     */
    Angle() = default;
    
    /**
     * @brief Constructor for Angle entity connecting three atoms.
     * @param atom1 Reference to the first atom.
     * @param atom2 Reference to the second atom (center).
     * @param atom3 Reference to the third atom.
     */
    Angle(const Atom& atom1, const Atom& atom2, const Atom& atom3);

    /**
     * @brief Virtual destructor.
     */
    virtual ~Angle() = default;

    /**
     * @brief Copy constructor.
     */
    Angle(const Angle&) = delete;

    /**
     * @brief Move constructor.
     */
    Angle(Angle&&) = default;

    /**
     * @brief Copy assignment operator.
     */
    Angle& operator=(const Angle&) = delete;

    /**
     * @brief Move assignment operator.
     */
    Angle& operator=(Angle&&) = default;
};

} // namespace molcpp



