#include <catch2/catch_test_macros.hpp>
#include "molcpp/ecs/ecs.hpp"

// Simple test components using basic types
struct TestPosition {
    float x, y, z;
    TestPosition() : x(0.0f), y(0.0f), z(0.0f) {}
    TestPosition(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct TestElement {
    std::string symbol;
    int atomic_number;
    TestElement() : symbol(""), atomic_number(0) {}
    TestElement(const std::string& sym, int num) : symbol(sym), atomic_number(num) {}
};

using namespace molcpp::ecs;

TEST_CASE("Entity creation and basic operations", "[entity]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Entity creation increases count") {
        auto entity = std::make_unique<Entity>();
        REQUIRE(system.entity_count() == initial_count + 1);
        REQUIRE(entity->get_id() > 0);
    }
    
    SECTION("Entity destruction decreases count") {
        {
            auto entity = std::make_unique<Entity>();
            REQUIRE(system.entity_count() == initial_count + 1);
        }
        REQUIRE(system.entity_count() == initial_count);
    }
    
    SECTION("Entities have unique IDs") {
        auto entity1 = std::make_unique<Entity>();
        auto entity2 = std::make_unique<Entity>();
        REQUIRE(entity1->get_id() != entity2->get_id());
    }
}

TEST_CASE("Entity component operations", "[entity][components]") {
    auto entity = std::make_unique<Entity>();
    
    SECTION("Adding components") {
        REQUIRE_FALSE(entity->has_component<TestPosition>());
        
        auto& pos = entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        REQUIRE(entity->has_component<TestPosition>());
        REQUIRE(pos.x == 1.0f);
        REQUIRE(pos.y == 2.0f);
        REQUIRE(pos.z == 3.0f);
    }
    
    SECTION("Getting components") {
        entity->add_component<TestPosition>(5.0f, 6.0f, 7.0f);
        
        auto* pos = entity->get_component<TestPosition>();
        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 5.0f);
        REQUIRE(pos->y == 6.0f);
        REQUIRE(pos->z == 7.0f);
        
        // Test const version
        const auto* const_entity = entity.get();
        const auto* const_pos = const_entity->get_component<TestPosition>();
        REQUIRE(const_pos != nullptr);
        REQUIRE(const_pos->x == 5.0f);
    }
    
    SECTION("Getting non-existent components") {
        auto* elem = entity->get_component<TestElement>();
        REQUIRE(elem == nullptr);
        REQUIRE_FALSE(entity->has_component<TestElement>());
    }
    
    SECTION("Multiple components") {
        entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        entity->add_component<TestElement>("H", 1);
        
        REQUIRE(entity->has_component<TestPosition>());
        REQUIRE(entity->has_component<TestElement>());
        
        auto* pos = entity->get_component<TestPosition>();
        auto* elem = entity->get_component<TestElement>();
        
        REQUIRE(pos != nullptr);
        REQUIRE(elem != nullptr);
        
        REQUIRE(elem->symbol == "H");
        REQUIRE(elem->atomic_number == 1);
    }
    
    SECTION("Removing components") {
        entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        entity->add_component<TestElement>("C", 6);
        
        REQUIRE(entity->has_component<TestPosition>());
        REQUIRE(entity->has_component<TestElement>());
        
        bool removed = entity->remove_component<TestPosition>();
        REQUIRE(removed);
        REQUIRE_FALSE(entity->has_component<TestPosition>());
        REQUIRE(entity->has_component<TestElement>());
        
        // Try to remove non-existent component
        bool removed_again = entity->remove_component<TestPosition>();
        REQUIRE_FALSE(removed_again);
    }
}

TEST_CASE("Entity move semantics", "[entity]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Move constructor") {
        EntityId original_id;
        {
            auto entity1 = std::make_unique<Entity>();
            original_id = entity1->get_id();
            entity1->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
            
            auto entity2 = std::make_unique<Entity>(std::move(*entity1));
            
            REQUIRE(entity2->get_id() == original_id);
            REQUIRE(entity2->has_component<TestPosition>());
            REQUIRE(system.entity_count() == initial_count + 1); // Should still be just one entity
            
            auto* pos = entity2->get_component<TestPosition>();
            REQUIRE(pos != nullptr);
            REQUIRE(pos->x == 1.0f);
        }
        
        REQUIRE(system.entity_count() == initial_count);
    }
}
