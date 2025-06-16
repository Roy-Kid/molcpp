#include "molcpp/ecs/entity.hpp"
#include <stdexcept>

namespace molcpp::ecs {

// Static member initialization
Entity::EntityId Entity::next_id_ = 1;

Entity::Entity() : id_(next_id_++) {
    System::instance().register_entity(this);
}

Entity::~Entity() {
    if (id_ != 0) { // Check if entity hasn't been moved
        System::instance().deregister_entity(this);
    }
}

Entity::Entity(Entity&& other) noexcept 
    : id_(other.id_), components_(std::move(other.components_)) {
    other.id_ = 0; // Mark the moved-from entity as invalid
    
    // Update the system's entity registry
    if (id_ != 0) {
        auto& system = System::instance();
        system.entities_[id_] = this;
        
        // Update component registries
        for (auto& [type, entity_set] : system.component_entities_) {
            if (entity_set.find(&other) != entity_set.end()) {
                entity_set.erase(&other);
                entity_set.insert(this);
            }
        }
    }
}

Entity& Entity::operator=(Entity&& other) noexcept {
    if (this != &other) {
        // Deregister current entity
        if (id_ != 0) {
            System::instance().deregister_entity(this);
        }
        
        // Move data
        id_ = other.id_;
        components_ = std::move(other.components_);
        other.id_ = 0;
        
        // Update the system's entity registry
        if (id_ != 0) {
            auto& system = System::instance();
            system.entities_[id_] = this;
            
            // Update component registries
            for (auto& [type, entity_set] : system.component_entities_) {
                if (entity_set.find(&other) != entity_set.end()) {
                    entity_set.erase(&other);
                    entity_set.insert(this);
                }
            }
        }
    }
    return *this;
}

} // namespace molcpp::ecs
