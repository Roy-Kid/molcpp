#pragma once

/**
 * @file atom.hpp
 * @brief Abstract Atom entity for molecular modeling.
 */

#include "molcpp/ecs/entity.hpp"

namespace molcpp {

/**
 * @brief Abstract Atom entity that inherits from Entity.
 * An atom is a basic building block of molecular structures.
 * Specific properties can be added via components.
 */
class Atom : public molcpp::ecs::Entity {
public:
    /**
     * @brief Default constructor for Atom entity.
     */
    Atom() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Atom() = default;

    /**
     * @brief Copy constructor.
     */
    Atom(const Atom&) = delete;

    /**
     * @brief Move constructor.
     */
    Atom(Atom&&) = default;

    /**
     * @brief Copy assignment operator.
     */
    Atom& operator=(const Atom&) = delete;

    /**
     * @brief Move assignment operator.
     */
    Atom& operator=(Atom&&) = default;
};

} // namespace molcpp
