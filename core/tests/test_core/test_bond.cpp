#include <catch2/catch_test_macros.hpp>
#include "molcpp/core/ecs/ecs.hpp"
#include "molcpp/core/atom.hpp"
#include "molcpp/core/bond.hpp"
#include "molcpp/core/ecs/components.hpp"

using namespace molcpp::ecs;
using namespace molcpp::ecs::components;

TEST_CASE("Bond entity creation and basic operations", "[bond]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Bond creation increases entity count") {
        auto bond = std::make_unique<molcpp::Bond>();
        REQUIRE(system.entity_count() == initial_count + 1);
        REQUIRE(bond->get_id() > 0);
    }
    
    SECTION("Bond destruction decreases entity count") {
        {
            auto bond = std::make_unique<molcpp::Bond>();
            REQUIRE(system.entity_count() == initial_count + 1);
        }
        REQUIRE(system.entity_count() == initial_count);
    }
    
    SECTION("Bonds have unique IDs") {
        auto bond1 = std::make_unique<molcpp::Bond>();
        auto bond2 = std::make_unique<molcpp::Bond>();
        REQUIRE(bond1->get_id() != bond2->get_id());
    }
}

TEST_CASE("Bond creation with atom references", "[bond][atom]") {
    auto& system = System::instance();
    size_t initial_count = system.entity_count();
    
    SECTION("Bond constructor with two atoms") {
        auto atom1 = std::make_unique<molcpp::Atom>();
        auto atom2 = std::make_unique<molcpp::Atom>();
        
        auto bond = std::make_unique<molcpp::Bond>(*atom1, *atom2);
        
        REQUIRE(system.entity_count() == initial_count + 3); // 2 atoms + 1 bond
        REQUIRE(bond->has_component<BondInfo>());
        
        auto* bond_info = bond->get_component<BondInfo>();
        REQUIRE(bond_info != nullptr);
        REQUIRE(bond_info->atom1_id == atom1->get_id());
        REQUIRE(bond_info->atom2_id == atom2->get_id());
    }
    
    SECTION("Multiple bonds between different atoms") {
        auto carbon = std::make_unique<molcpp::Atom>();
        auto hydrogen1 = std::make_unique<molcpp::Atom>();
        auto hydrogen2 = std::make_unique<molcpp::Atom>();
        
        // Add some components to identify the atoms
        carbon->add_component<Element>("C", 6);
        hydrogen1->add_component<Element>("H", 1);
        hydrogen2->add_component<Element>("H", 1);
        
        auto bond1 = std::make_unique<molcpp::Bond>(*carbon, *hydrogen1);
        auto bond2 = std::make_unique<molcpp::Bond>(*carbon, *hydrogen2);
        
        REQUIRE(bond1->get_id() != bond2->get_id());
        
        auto* bond1_info = bond1->get_component<BondInfo>();
        auto* bond2_info = bond2->get_component<BondInfo>();
        
        REQUIRE(bond1_info != nullptr);
        REQUIRE(bond2_info != nullptr);
        
        // Both bonds should connect to carbon
        REQUIRE(bond1_info->atom1_id == carbon->get_id());
        REQUIRE(bond2_info->atom1_id == carbon->get_id());
        
        // But to different hydrogen atoms
        REQUIRE(bond1_info->atom2_id == hydrogen1->get_id());
        REQUIRE(bond2_info->atom2_id == hydrogen2->get_id());
        REQUIRE(bond1_info->atom2_id != bond2_info->atom2_id);
    }
}

TEST_CASE("Component base class functionality", "[components]") {
    SECTION("Component type information") {
        Position pos(1.0, 2.0, 3.0);
        Element elem("C", 6);
        BondInfo bond_info(1, 2);
        
        REQUIRE(pos.get_type_name() == "Position");
        REQUIRE(elem.get_type_name() == "Element");
        REQUIRE(bond_info.get_type_name() == "BondInfo");
        
        REQUIRE(pos.get_type() == typeid(Position));
        REQUIRE(elem.get_type() == typeid(Element));
        REQUIRE(bond_info.get_type() == typeid(BondInfo));
    }
}

TEST_CASE("System querying with bonds", "[bond][system]") {
    auto& system = System::instance();
    
    SECTION("Query entities by BondInfo component") {
        auto atom1 = std::make_unique<molcpp::Atom>();
        auto atom2 = std::make_unique<molcpp::Atom>();
        auto atom3 = std::make_unique<molcpp::Atom>();
        
        auto bond1 = std::make_unique<molcpp::Bond>(*atom1, *atom2);
        auto bond2 = std::make_unique<molcpp::Bond>(*atom2, *atom3);
        
        // Query entities with BondInfo component
        auto bond_entities = system.query<BondInfo>();
        REQUIRE(bond_entities.size() >= 2);
        
        // Check that our bonds are in the results
        bool found_bond1 = false;
        bool found_bond2 = false;
        
        for (auto* entity : bond_entities) {
            if (entity == bond1.get()) found_bond1 = true;
            if (entity == bond2.get()) found_bond2 = true;
        }
        
        REQUIRE(found_bond1);
        REQUIRE(found_bond2);
    }
}
