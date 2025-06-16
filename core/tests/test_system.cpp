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

struct TestVelocity {
    float vx, vy, vz;
    TestVelocity() : vx(0.0f), vy(0.0f), vz(0.0f) {}
    TestVelocity(float vx_, float vy_, float vz_) : vx(vx_), vy(vy_), vz(vz_) {}
};

using namespace molcpp::ecs;

TEST_CASE("System singleton behavior", "[system]") {
    auto& system1 = System::instance();
    auto& system2 = System::instance();
    
    REQUIRE(&system1 == &system2);
}

TEST_CASE("System entity registration", "[system]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Automatic registration on entity creation") {
        auto entity = std::make_unique<Entity>();
        REQUIRE(system.entity_count() == initial_count + 1);
    }
    
    SECTION("Automatic deregistration on entity destruction") {
        {
            auto entity = std::make_unique<Entity>();
            REQUIRE(system.entity_count() == initial_count + 1);
        }
        REQUIRE(system.entity_count() == initial_count);
    }
    
    SECTION("Multiple entities") {
        std::vector<std::unique_ptr<Entity>> entities;
        
        for (int i = 0; i < 5; ++i) {
            entities.push_back(std::make_unique<Entity>());
        }
        
        REQUIRE(system.entity_count() == initial_count + 5);
        
        entities.clear();
        REQUIRE(system.entity_count() == initial_count);
    }
}

TEST_CASE("System component querying", "[system][components]") {
    auto& system = System::instance();
    
    SECTION("Empty queries") {
        auto position_entities = system.query<TestPosition>();
        auto element_entities = system.query<TestElement>();
        
        // Note: These might not be empty due to other tests, so we just check the behavior
        REQUIRE(position_entities.size() == system.component_count<TestPosition>());
        REQUIRE(element_entities.size() == system.component_count<TestElement>());
    }
    
    SECTION("Querying entities with components") {
        auto entity1 = std::make_unique<Entity>();
        auto entity2 = std::make_unique<Entity>();
        auto entity3 = std::make_unique<Entity>();
        
        // Add Position to entity1 and entity2
        entity1->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        entity2->add_component<TestPosition>(4.0f, 5.0f, 6.0f);
        
        // Add Element to entity2 and entity3
        entity2->add_component<TestElement>("H", 1);
        entity3->add_component<TestElement>("O", 8);
        
        auto position_entities = system.query<TestPosition>();
        auto element_entities = system.query<TestElement>();
        
        // Check that the right entities are returned
        bool found_entity1 = false, found_entity2_pos = false, found_entity2_elem = false, found_entity3 = false;
        
        for (auto* entity : position_entities) {
            if (entity == entity1.get()) found_entity1 = true;
            if (entity == entity2.get()) found_entity2_pos = true;
        }
        
        for (auto* entity : element_entities) {
            if (entity == entity2.get()) found_entity2_elem = true;
            if (entity == entity3.get()) found_entity3 = true;
        }
        
        REQUIRE(found_entity1);
        REQUIRE(found_entity2_pos);
        REQUIRE(found_entity2_elem);
        REQUIRE(found_entity3);
        
        // Test component counts
        REQUIRE(system.component_count<TestPosition>() >= 2);
        REQUIRE(system.component_count<TestElement>() >= 2);
    }
    
    SECTION("Const querying") {
        auto entity = std::make_unique<Entity>();
        entity->add_component<TestVelocity>(1.0f, 2.0f, 3.0f);
        
        const auto& const_system = system;
        auto velocity_entities = const_system.query<TestVelocity>();
        
        REQUIRE(velocity_entities.size() >= 1);
        REQUIRE(const_system.component_count<TestVelocity>() >= 1);
    }
}

TEST_CASE("System component registration and deregistration", "[system][components]") {
    auto& system = System::instance();
    
    SECTION("Component registration updates query results") {
        size_t initial_position_count = system.component_count<TestPosition>();
        
        auto entity = std::make_unique<Entity>();
        entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        
        REQUIRE(system.component_count<TestPosition>() == initial_position_count + 1);
        
        auto position_entities = system.query<TestPosition>();
        bool found = false;
        for (auto* e : position_entities) {
            if (e == entity.get()) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }
    
    SECTION("Component removal updates query results") {
        auto entity = std::make_unique<Entity>();
        entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
        
        size_t count_with_component = system.component_count<TestPosition>();
        
        entity->remove_component<TestPosition>();
        
        REQUIRE(system.component_count<TestPosition>() == count_with_component - 1);
        
        auto position_entities = system.query<TestPosition>();
        for (auto* e : position_entities) {
            REQUIRE(e != entity.get());
        }
    }
    
    SECTION("Entity destruction removes from all component queries") {
        size_t initial_pos_count = system.component_count<TestPosition>();
        size_t initial_elem_count = system.component_count<TestElement>();
        
        {
            auto entity = std::make_unique<Entity>();
            entity->add_component<TestPosition>(1.0f, 2.0f, 3.0f);
            entity->add_component<TestElement>("C", 6);
            
            REQUIRE(system.component_count<TestPosition>() == initial_pos_count + 1);
            REQUIRE(system.component_count<TestElement>() == initial_elem_count + 1);
        }
        
        REQUIRE(system.component_count<TestPosition>() == initial_pos_count);
        REQUIRE(system.component_count<TestElement>() == initial_elem_count);
    }
}
