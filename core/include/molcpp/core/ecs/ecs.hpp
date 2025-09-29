#pragma once

/**
 * @file ecs.hpp
 * @brief Main header for the molcpp ECS framework.
 * 
 * This header includes all the necessary components for using the
 * Entity-Component-System framework in molcpp.
 */

#include "molcpp/core/ecs/entity.hpp"
#include "molcpp/core/ecs/system.hpp"

namespace molcpp::ecs {

// Type aliases for convenience
using EntityId = Entity::EntityId;

} // namespace molcpp::ecs
