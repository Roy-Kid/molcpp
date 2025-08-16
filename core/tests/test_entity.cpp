#include <catch2/catch_test_macros.hpp>
#include "molcpp/ecs/ecs.hpp"
#include "molcpp/atom.hpp"
#include "molcpp/ecs/components.hpp"

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

TEST_CASE("Atom entity creation and basic operations", "[atom]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Atom creation increases entity count") {
        auto atom = std::make_unique<molcpp::Atom>();
        REQUIRE(system.entity_count() == initial_count + 1);
        REQUIRE(atom->get_id() > 0);
    }
    
    SECTION("Atom destruction decreases entity count") {
        {
            auto atom = std::make_unique<molcpp::Atom>();
            REQUIRE(system.entity_count() == initial_count + 1);
        }
        REQUIRE(system.entity_count() == initial_count);
    }
    
    SECTION("Atoms have unique IDs") {
        auto atom1 = std::make_unique<molcpp::Atom>();
        auto atom2 = std::make_unique<molcpp::Atom>();
        REQUIRE(atom1->get_id() != atom2->get_id());
    }
}

TEST_CASE("Atom component operations", "[atom][components]") {
    using namespace molcpp::ecs::components;
    
    auto atom = std::make_unique<molcpp::Atom>();
    
    SECTION("Adding basic atomic components") {
        REQUIRE_FALSE(atom->has_component<Position>());
        REQUIRE_FALSE(atom->has_component<Element>());
        REQUIRE_FALSE(atom->has_component<Radius>());
        
        // Add position component
        auto& pos = atom->add_component<Position>(1.5, 2.5, 3.5);
        REQUIRE(atom->has_component<Position>());
        REQUIRE(pos.x == 1.5);
        REQUIRE(pos.y == 2.5);
        REQUIRE(pos.z == 3.5);
        
        // Add element component
        auto& elem = atom->add_component<Element>("C", 6);
        REQUIRE(atom->has_component<Element>());
        REQUIRE(elem.symbol == "C");
        REQUIRE(elem.atomic_number == 6);
        
        // Add radius component
        auto& radius = atom->add_component<Radius>(0.77);
        REQUIRE(atom->has_component<Radius>());
        REQUIRE(radius.value == 0.77);
    }
    
    SECTION("Creating a complete carbon atom") {
        atom->add_component<Position>(0.0, 0.0, 0.0);
        atom->add_component<Element>("C", 6);
        atom->add_component<Radius>(0.77);
        atom->add_component<Mass>(12.011);
        atom->add_component<Charge>(0.0);
        
        REQUIRE(atom->has_component<Position>());
        REQUIRE(atom->has_component<Element>());
        REQUIRE(atom->has_component<Radius>());
        REQUIRE(atom->has_component<Mass>());
        REQUIRE(atom->has_component<Charge>());
        
        // Verify component values
        auto* element = atom->get_component<Element>();
        REQUIRE(element != nullptr);
        REQUIRE(element->symbol == "C");
        REQUIRE(element->atomic_number == 6);
        
        auto* mass = atom->get_component<Mass>();
        REQUIRE(mass != nullptr);
        REQUIRE(mass->value == 12.011);
    }
    
    SECTION("Creating different types of atoms") {
        // Hydrogen atom
        auto hydrogen = std::make_unique<molcpp::Atom>();
        hydrogen->add_component<Position>(1.0, 0.0, 0.0);
        hydrogen->add_component<Element>("H", 1);
        hydrogen->add_component<Radius>(0.31);
        hydrogen->add_component<Mass>(1.008);
        
        // Oxygen atom
        auto oxygen = std::make_unique<molcpp::Atom>();
        oxygen->add_component<Position>(-1.0, 0.0, 0.0);
        oxygen->add_component<Element>("O", 8);
        oxygen->add_component<Radius>(0.66);
        oxygen->add_component<Mass>(15.999);
        
        // Verify both atoms exist and have different properties
        REQUIRE(hydrogen->get_id() != oxygen->get_id());
        
        auto* h_elem = hydrogen->get_component<Element>();
        auto* o_elem = oxygen->get_component<Element>();
        REQUIRE(h_elem->symbol == "H");
        REQUIRE(o_elem->symbol == "O");
        REQUIRE(h_elem->atomic_number == 1);
        REQUIRE(o_elem->atomic_number == 8);
    }
    
    SECTION("Atom velocity and dynamics components") {
        atom->add_component<Position>(0.0, 0.0, 0.0);
        atom->add_component<Velocity>(0.1, 0.2, 0.3);
        atom->add_component<Mass>(12.011);
        
        auto* velocity = atom->get_component<Velocity>();
        REQUIRE(velocity != nullptr);
        REQUIRE(velocity->vx == 0.1);
        REQUIRE(velocity->vy == 0.2);
        REQUIRE(velocity->vz == 0.3);
        
        // Modify velocity
        velocity->vx = 0.5;
        REQUIRE(atom->get_component<Velocity>()->vx == 0.5);
    }
    
    SECTION("Removing atom components") {
        atom->add_component<Position>(1.0, 2.0, 3.0);
        atom->add_component<Element>("N", 7);
        atom->add_component<Velocity>(0.1, 0.1, 0.1);
        
        REQUIRE(atom->has_component<Position>());
        REQUIRE(atom->has_component<Element>());
        REQUIRE(atom->has_component<Velocity>());
        
        // Remove velocity component
        bool removed = atom->remove_component<Velocity>();
        REQUIRE(removed);
        REQUIRE_FALSE(atom->has_component<Velocity>());
        REQUIRE(atom->has_component<Position>());
        REQUIRE(atom->has_component<Element>());
        
        // Try to remove non-existent component
        bool removed_again = atom->remove_component<Velocity>();
        REQUIRE_FALSE(removed_again);
    }
}

TEST_CASE("Atom move semantics", "[atom]") {
    using namespace molcpp::ecs::components;
    
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Move constructor preserves atom data") {
        EntityId original_id;
        {
            auto atom1 = std::make_unique<molcpp::Atom>();
            original_id = atom1->get_id();
            atom1->add_component<Position>(1.0, 2.0, 3.0);
            atom1->add_component<Element>("C", 6);
            atom1->add_component<Mass>(12.011);
            
            auto atom2 = std::make_unique<molcpp::Atom>(std::move(*atom1));
            
            REQUIRE(atom2->get_id() == original_id);
            REQUIRE(atom2->has_component<Position>());
            REQUIRE(atom2->has_component<Element>());
            REQUIRE(atom2->has_component<Mass>());
            REQUIRE(system.entity_count() == initial_count + 1); // Should still be just one entity
            
            // Verify component data is preserved
            auto* pos = atom2->get_component<Position>();
            auto* elem = atom2->get_component<Element>();
            auto* mass = atom2->get_component<Mass>();
            
            REQUIRE(pos != nullptr);
            REQUIRE(pos->x == 1.0);
            REQUIRE(pos->y == 2.0);
            REQUIRE(pos->z == 3.0);
            
            REQUIRE(elem != nullptr);
            REQUIRE(elem->symbol == "C");
            REQUIRE(elem->atomic_number == 6);
            
            REQUIRE(mass != nullptr);
            REQUIRE(mass->value == 12.011);
        }
        
        REQUIRE(system.entity_count() == initial_count);
    }
}

TEST_CASE("System querying with atoms", "[atom][system]") {
    using namespace molcpp::ecs::components;
    
    auto& system = System::instance();
    
    SECTION("Query atoms by component type") {
        // Create multiple atoms with different components
        auto carbon = std::make_unique<molcpp::Atom>();
        carbon->add_component<Position>(0.0, 0.0, 0.0);
        carbon->add_component<Element>("C", 6);
        
        auto hydrogen1 = std::make_unique<molcpp::Atom>();
        hydrogen1->add_component<Position>(1.0, 0.0, 0.0);
        hydrogen1->add_component<Element>("H", 1);
        
        auto hydrogen2 = std::make_unique<molcpp::Atom>();
        hydrogen2->add_component<Position>(-1.0, 0.0, 0.0);
        hydrogen2->add_component<Element>("H", 1);
        
        // Query entities with Position component
        auto position_entities = system.query<Position>();
        REQUIRE(position_entities.size() >= 3);
        
        // Query entities with Element component
        auto element_entities = system.query<Element>();
        REQUIRE(element_entities.size() >= 3);
        
        // Check that our atoms are in the results
        bool found_carbon = false;
        bool found_hydrogen1 = false;
        bool found_hydrogen2 = false;
        
        for (auto* entity : element_entities) {
            if (entity == carbon.get()) found_carbon = true;
            if (entity == hydrogen1.get()) found_hydrogen1 = true;
            if (entity == hydrogen2.get()) found_hydrogen2 = true;
        }
        
        REQUIRE(found_carbon);
        REQUIRE(found_hydrogen1);
        REQUIRE(found_hydrogen2);
    }
}
