#pragma once

/**
 * @file bond.hpp
 * @brief Abstract Bond entity for molecular modeling.
 */

#include "molcpp/ecs/entity.hpp"

namespace molcpp {

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
