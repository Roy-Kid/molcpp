#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <any>

namespace molcpp::ecs {

class System; // Forward declaration

/**
 * @brief Base class for all entities in the ECS system.
 * 
 * Entities are automatically registered with the global System instance
 * upon construction and deregistered upon destruction.
 */
class Entity {
public:
    using EntityId = size_t;

    /**
     * @brief Construct a new Entity and register it with the global System.
     */
    Entity();

    /**
     * @brief Destroy the Entity and deregister it from the global System.
     */
    virtual ~Entity();

    // Disable copy constructor and assignment operator
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Enable move constructor and assignment operator
    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

    /**
     * @brief Get the unique ID of this entity.
     * @return EntityId The unique identifier for this entity.
     */
    EntityId get_id() const { return id_; }

    /**
     * @brief Add a component to this entity.
     * @tparam T The component type.
     * @tparam Args Constructor argument types.
     * @param args Constructor arguments for the component.
     * @return T& Reference to the added component.
     */
    template<typename T, typename... Args>
    T& add_component(Args&&... args);

    /**
     * @brief Get a component from this entity.
     * @tparam T The component type.
     * @return T* Pointer to the component, or nullptr if not found.
     */
    template<typename T>
    T* get_component();

    /**
     * @brief Get a component from this entity (const version).
     * @tparam T The component type.
     * @return const T* Pointer to the component, or nullptr if not found.
     */
    template<typename T>
    const T* get_component() const;

    /**
     * @brief Check if this entity has a component of the given type.
     * @tparam T The component type.
     * @return true if the component exists, false otherwise.
     */
    template<typename T>
    bool has_component() const;

    /**
     * @brief Remove a component from this entity.
     * @tparam T The component type.
     * @return true if the component was removed, false if it didn't exist.
     */
    template<typename T>
    bool remove_component();

private:
    EntityId id_;
    std::unordered_map<std::type_index, std::any> components_;
    
    static EntityId next_id_;
    
    friend class System;
};

} // namespace molcpp::ecs

// Include system.hpp after Entity class declaration to avoid circular dependency
#include "system.hpp"

namespace molcpp::ecs {

// Template function implementations
template<typename T, typename... Args>
inline T& Entity::add_component(Args&&... args) {
    std::type_index type_idx(typeid(T));
    
    // Create and store the component
    components_[type_idx] = T(std::forward<Args>(args)...);
    
    // Register with the system
    System::instance().register_component<T>(id_, this);
    
    return std::any_cast<T&>(components_[type_idx]);
}

template<typename T>
inline T* Entity::get_component() {
    std::type_index type_idx(typeid(T));
    auto it = components_.find(type_idx);
    if (it != components_.end()) {
        return &std::any_cast<T&>(it->second);
    }
    return nullptr;
}

template<typename T>
inline const T* Entity::get_component() const {
    std::type_index type_idx(typeid(T));
    auto it = components_.find(type_idx);
    if (it != components_.end()) {
        return &std::any_cast<const T&>(it->second);
    }
    return nullptr;
}

template<typename T>
inline bool Entity::has_component() const {
    std::type_index type_idx(typeid(T));
    return components_.find(type_idx) != components_.end();
}

template<typename T>
inline bool Entity::remove_component() {
    std::type_index type_idx(typeid(T));
    auto it = components_.find(type_idx);
    if (it != components_.end()) {
        components_.erase(it);
        System::instance().deregister_component<T>(id_);
        return true;
    }
    return false;
}

} // namespace molcpp::ecs
