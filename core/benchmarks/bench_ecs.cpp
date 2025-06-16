#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "molcpp/ecs/ecs.hpp"
#include "molcpp/atom.hpp"
#include <vector>
#include <memory>

using namespace molcpp;

// Test components for performance testing
struct TestPosition {
    float x, y, z;
    TestPosition(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

struct TestElement {
    std::string symbol;
    int atomic_number;
    TestElement(const std::string& sym = "", int num = 0) : symbol(sym), atomic_number(num) {}
};

struct TestRadius {
    float value;
    TestRadius(float v = 1.0f) : value(v) {}
};

TEST_CASE("ECS Performance with Abstract Entities", "[performance][ecs]") {
    auto& system = ecs::System::instance();
    
    SECTION("Entity creation performance") {
        const int num_entities = 1000;
        std::vector<std::unique_ptr<Atom>> atoms;
        atoms.reserve(static_cast<size_t>(num_entities));
        
        BENCHMARK("Create 1K abstract atoms") {
            atoms.clear();
            for (int i = 0; i < num_entities; ++i) {
                atoms.push_back(std::make_unique<Atom>());
            }
            return atoms.size();
        };
    }
    
    SECTION("Component addition performance") {
        const size_t num_entities = 1000;
        std::vector<std::unique_ptr<Atom>> entities;
        entities.reserve(num_entities);
        
        // Pre-create entities
        for (size_t i = 0; i < num_entities; ++i) {
            entities.push_back(std::make_unique<Atom>());
        }
        
        BENCHMARK("Add Position components to 1K entities") {
            for (size_t i = 0; i < entities.size(); ++i) {
                entities[i]->add_component<TestPosition>(static_cast<float>(i) * 1.0f, static_cast<float>(i) * 2.0f, static_cast<float>(i) * 3.0f);
            }
            return entities.size();
        };
        
        BENCHMARK("Add Element components to 1K entities") {
            for (size_t i = 0; i < entities.size(); ++i) {
                entities[i]->add_component<TestElement>("C", 6);
            }
            return entities.size();
        };
    }
    
    SECTION("Query performance") {
        // Setup: Create entities with components
        const int num_entities = 1000;
        std::vector<std::unique_ptr<Atom>> atoms;
        atoms.reserve(static_cast<size_t>(num_entities));
        
        for (int i = 0; i < num_entities; ++i) {
            atoms.push_back(std::make_unique<Atom>());
            atoms.back()->add_component<TestPosition>(static_cast<float>(i) * 1.0f, static_cast<float>(i) * 2.0f, static_cast<float>(i) * 3.0f);
            atoms.back()->add_component<TestElement>("C", 6);
            atoms.back()->add_component<TestRadius>(0.77f);
        }
        
        BENCHMARK("Query 1K entities by Position") {
            auto result = system.query<TestPosition>();
            return result.size();
        };
        
        BENCHMARK("Query 1K entities by Element") {
            auto result = system.query<TestElement>();
            return result.size();
        };
        
        BENCHMARK("Component count queries") {
            size_t total = 0;
            total += system.component_count<TestPosition>();
            total += system.component_count<TestElement>();
            total += system.component_count<TestRadius>();
            return total;
        };
    }
    
    SECTION("Large scale performance") {
        const int num_entities = 10000;
        std::vector<std::unique_ptr<Atom>> atoms;
        atoms.reserve(static_cast<size_t>(num_entities));
        
        BENCHMARK("Create 10K abstract atoms") {
            atoms.clear();
            for (int i = 0; i < num_entities; ++i) {
                atoms.push_back(std::make_unique<Atom>());
            }
            return atoms.size();
        };
    }
    
    SECTION("Large scale queries") {
        // Setup: Create 100K entities
        const int num_entities = 100000;
        std::vector<std::unique_ptr<Atom>> atoms;
        atoms.reserve(static_cast<size_t>(num_entities));
        
        for (int i = 0; i < num_entities; ++i) {
            atoms.push_back(std::make_unique<Atom>());
            atoms.back()->add_component<TestPosition>(static_cast<float>(i) * 1.0f, static_cast<float>(i) * 2.0f, static_cast<float>(i) * 3.0f);
        }
        
        BENCHMARK("Query 100K entities by Position") {
            auto result = system.query<TestPosition>();
            return result.size();
        };
        
        BENCHMARK("Mixed queries on 100K entities") {
            auto pos_entities = system.query<TestPosition>();
            auto elem_entities = system.query<TestElement>();
            auto radius_entities = system.query<TestRadius>();
            return pos_entities.size() + elem_entities.size() + radius_entities.size();
        };
    }
    
    SECTION("Component access performance") {
        const int num_entities = 10000;
        std::vector<std::unique_ptr<Atom>> atoms;
        atoms.reserve(static_cast<size_t>(num_entities));
        
        for (int i = 0; i < num_entities; ++i) {
            atoms.push_back(std::make_unique<Atom>());
            atoms.back()->add_component<TestPosition>(static_cast<float>(i) * 1.0f, static_cast<float>(i) * 2.0f, static_cast<float>(i) * 3.0f);
            atoms.back()->add_component<TestRadius>(0.77f);
        }
        
        BENCHMARK("Access Position components") {
            float total = 0.0f;
            for (const auto& atom : atoms) {
                auto* pos = atom->get_component<TestPosition>();
                if (pos) {
                    total += pos->x + pos->y + pos->z;
                }
            }
            return total;
        };
        
        BENCHMARK("Modify Position and Radius components") {
            for (const auto& atom : atoms) {
                auto* pos = atom->get_component<TestPosition>();
                auto* radius = atom->get_component<TestRadius>();
                if (pos && radius) {
                    pos->x += 1.0f;
                    radius->value *= 1.1f;
                }
            }
            return atoms.size();
        };
        
        BENCHMARK("Complex component checks and access") {
            size_t count = 0;
            for (const auto& atom : atoms) {
                if (atom->has_component<TestPosition>() &&
                    atom->has_component<TestElement>() &&
                    atom->has_component<TestRadius>()) {
                    count++;
                }
            }
            return count;
        };
    }
}
