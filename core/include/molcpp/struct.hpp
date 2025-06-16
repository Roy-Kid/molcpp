#pragma once

/**
 * @file molecule.hpp
 * @brief Abstract Struct entity for molecular modeling.
 */

#include "molcpp/ecs/entity.hpp"

namespace molcpp {

/**
 * @brief Abstract Struct entity representing a molecular structure.
 * A struct is a collection of entities and their relationships.
 * Specific properties can be added via components.
 */
class Struct : public molcpp::ecs::Entity {
public:
    /**
     * @brief Default constructor for Struct entity.
     */
    Struct() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Struct() = default;

    /**
     * @brief Copy constructor.
     */
    Struct(const Struct&) = delete;

    /**
     * @brief Move constructor.
     */
    Struct(Struct&&) = default;

    /**
     * @brief Copy assignment operator.
     */
    Struct& operator=(const Struct&) = delete;

    /**
     * @brief Move assignment operator.
     */
    Struct& operator=(Struct&&) = default;
};

} // namespace molcpp
