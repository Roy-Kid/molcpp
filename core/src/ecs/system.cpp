#include "molcpp/ecs/system.hpp"
#include "molcpp/ecs/entity.hpp"

namespace molcpp::ecs {

System& System::instance() {
    static System instance_;
    return instance_;
}

void System::register_entity(Entity* entity) {
    if (entity) {
        entities_[entity->get_id()] = entity;
    }
}

void System::deregister_entity(Entity* entity) {
    if (entity) {
        EntityId id = entity->get_id();
        
        // Remove from main entity registry
        entities_.erase(id);
        
        // Remove from all component registries
        for (auto& [type, entity_set] : component_entities_) {
            entity_set.erase(entity);
        }
    }
}

} // namespace molcpp::ecs