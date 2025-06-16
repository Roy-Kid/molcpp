#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <unordered_set>
#include <vector>

namespace molcpp::ecs {

class Entity; // Forward declaration

/**
 * @brief Singleton System class that manages all entities and components in the ECS.
 * 
 * The System maintains registries of entities by component type and provides
 * querying capabilities. It is automatically initialized and should be accessed
 * via System::instance().
 */
class System {
public:
    using EntityId = size_t;

    /**
     * @brief Get the singleton instance of the System.
     * @return System& Reference to the singleton System instance.
     */
    static System& instance();

    /**
     * @brief Query all entities that have a component of the given type.
     * @tparam T The component type to query for.
     * @return std::vector<Entity*> Vector of pointers to entities with the component.
     */
    template<typename T>
    std::vector<Entity*> query() {
        std::type_index type_idx(typeid(T));
        std::vector<Entity*> result;
        
        auto it = component_entities_.find(type_idx);
        if (it != component_entities_.end()) {
            result.reserve(it->second.size());
            for (Entity* entity : it->second) {
                result.push_back(entity);
            }
        }
        
        return result;
    }

    /**
     * @brief Query all entities that have a component of the given type (const version).
     * @tparam T The component type to query for.
     * @return std::vector<const Entity*> Vector of const pointers to entities with the component.
     */
    template<typename T>
    std::vector<const Entity*> query() const {
        std::type_index type_idx(typeid(T));
        std::vector<const Entity*> result;
        
        auto it = component_entities_.find(type_idx);
        if (it != component_entities_.end()) {
            result.reserve(it->second.size());
            for (const Entity* entity : it->second) {
                result.push_back(entity);
            }
        }
        
        return result;
    }

    /**
     * @brief Get the total number of registered entities.
     * @return size_t The number of entities.
     */
    size_t entity_count() const { return entities_.size(); }

    /**
     * @brief Get the number of entities with a specific component type.
     * @tparam T The component type.
     * @return size_t The number of entities with the component.
     */
    template<typename T>
    size_t component_count() const {
        std::type_index type_idx(typeid(T));
        auto it = component_entities_.find(type_idx);
        if (it != component_entities_.end()) {
            return it->second.size();
        }
        return 0;
    }

private:
    System() = default;
    ~System() = default;

    // Disable copy and move operations
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) = delete;
    System& operator=(System&&) = delete;

    /**
     * @brief Register an entity with the system.
     * @param entity Pointer to the entity to register.
     */
    void register_entity(Entity* entity);

    /**
     * @brief Deregister an entity from the system.
     * @param entity Pointer to the entity to deregister.
     */
    void deregister_entity(Entity* entity);

    /**
     * @brief Register a component for an entity.
     * @tparam T The component type.
     * @param entity_id The ID of the entity.
     * @param entity Pointer to the entity.
     */
    template<typename T>
    void register_component(EntityId /*entity_id*/, Entity* entity) {
        std::type_index type_idx(typeid(T));
        component_entities_[type_idx].insert(entity);
    }

    /**
     * @brief Deregister a component for an entity.
     * @tparam T The component type.
     * @param entity_id The ID of the entity.
     */
    template<typename T>
    void deregister_component(EntityId entity_id) {
        std::type_index type_idx(typeid(T));
        auto comp_it = component_entities_.find(type_idx);
        if (comp_it != component_entities_.end()) {
            auto entity_it = entities_.find(entity_id);
            if (entity_it != entities_.end()) {
                comp_it->second.erase(entity_it->second);
            }
        }
    }

    // Storage for all entities by ID
    std::unordered_map<EntityId, Entity*> entities_;
    
    // Storage for entities by component type
    std::unordered_map<std::type_index, std::unordered_set<Entity*>> component_entities_;

    friend class Entity;
};

} // namespace molcpp::ecs
