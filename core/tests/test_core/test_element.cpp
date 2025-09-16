#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <molcpp/core/element.hpp>

using namespace molcpp;
using Catch::Approx;

TEST_CASE("Element class functionality", "[element]") {
    
    SECTION("Element initialization") {
        Element::initialize();
        REQUIRE(Element::count() > 0);
    }
    
    SECTION("Element creation by symbol") {
        Element::initialize();
        
        auto carbon = Element::create("C");
        REQUIRE(carbon.symbol == "C");
        REQUIRE(carbon.name == "carbon");
        REQUIRE(carbon.number == 6);
        REQUIRE(carbon.mass == Approx(12.01078 * daltons));
        
        auto hydrogen = Element::create("H");
        REQUIRE(hydrogen.symbol == "H");
        REQUIRE(hydrogen.name == "hydrogen");
        REQUIRE(hydrogen.number == 1);
        REQUIRE(hydrogen.mass == Approx(1.007947 * daltons));
        
        auto oxygen = Element::create("O");
        REQUIRE(oxygen.symbol == "O");
        REQUIRE(oxygen.name == "oxygen");
        REQUIRE(oxygen.number == 8);
        REQUIRE(oxygen.mass == Approx(15.99943 * daltons));
    }
    
    SECTION("Element creation by name") {
        Element::initialize();
        
        auto nitrogen = Element::create("nitrogen");
        REQUIRE(nitrogen.symbol == "N");
        REQUIRE(nitrogen.name == "nitrogen");
        REQUIRE(nitrogen.number == 7);
        
        auto sodium = Element::create("sodium");
        REQUIRE(sodium.symbol == "Na");
        REQUIRE(sodium.name == "sodium");
        REQUIRE(sodium.number == 11);
    }
    
    SECTION("Element creation by atomic number") {
        Element::initialize();
        
        auto iron = Element::create(26);
        REQUIRE(iron.symbol == "Fe");
        REQUIRE(iron.name == "iron");
        REQUIRE(iron.number == 26);
        
        auto zinc = Element::create(30);
        REQUIRE(zinc.symbol == "Zn");
        REQUIRE(zinc.name == "zinc");
        REQUIRE(zinc.number == 30);
    }
    
    SECTION("Case insensitive search") {
        Element::initialize();
        
        auto carbon_lower = Element::create("c");
        REQUIRE(carbon_lower.symbol == "C");
        
        auto carbon_upper = Element::create("CARBON");
        REQUIRE(carbon_upper.symbol == "C");
        
        auto hydrogen_mixed = Element::create("HyDrOgEn");
        REQUIRE(hydrogen_mixed.symbol == "H");
    }
    
    SECTION("Unknown element handling") {
        Element::initialize();
        
        auto unknown = Element::create("X");
        REQUIRE(unknown.symbol == "X");
        REQUIRE(unknown.name == "unknown");
        REQUIRE(unknown.number == 0);
        REQUIRE(unknown.mass == Approx(0.0 * daltons));
    }
    
    SECTION("Element existence checking") {
        Element::initialize();
        
        REQUIRE(Element::exists("C") == true);
        REQUIRE(Element::exists("H") == true);
        REQUIRE(Element::exists("O") == true);
        REQUIRE(Element::exists("Xx") == false);
        REQUIRE(Element::exists("") == false);
        
        REQUIRE(Element::exists(1) == true);
        REQUIRE(Element::exists(6) == true);
        REQUIRE(Element::exists(8) == true);
        REQUIRE(Element::exists(999) == false);
        REQUIRE(Element::exists(0) == true); // Unknown element
    }
    
    SECTION("Get symbols from identifiers") {
        Element::initialize();
        
        std::vector<std::string> element_symbols = {"C", "H", "O", "N"};
        auto symbols = Element::get_symbols(element_symbols);
        REQUIRE(symbols.size() == 4);
        REQUIRE(symbols[0] == "C");
        REQUIRE(symbols[1] == "H");
        REQUIRE(symbols[2] == "O");
        REQUIRE(symbols[3] == "N");
        
        std::vector<int> atomic_numbers = {1, 6, 8, 7};
        auto symbols_from_numbers = Element::get_symbols(atomic_numbers);
        REQUIRE(symbols_from_numbers.size() == 4);
        REQUIRE(symbols_from_numbers[0] == "H");
        REQUIRE(symbols_from_numbers[1] == "C");
        REQUIRE(symbols_from_numbers[2] == "O");
        REQUIRE(symbols_from_numbers[3] == "N");
    }
    
    SECTION("Get atomic number from symbol") {
        Element::initialize();
        
        REQUIRE(Element::get_atomic_number("H") == 1);
        REQUIRE(Element::get_atomic_number("C") == 6);
        REQUIRE(Element::get_atomic_number("O") == 8);
        REQUIRE(Element::get_atomic_number("Fe") == 26);
        REQUIRE(Element::get_atomic_number("Zn") == 30);
    }
    
    SECTION("Error handling for non-existent elements") {
        Element::initialize();
        
        REQUIRE_THROWS_AS(Element::create("Xx"), std::runtime_error);
        REQUIRE_THROWS_AS(Element::create(999), std::runtime_error);
        REQUIRE_THROWS_AS(Element::get_atomic_number("Xx"), std::runtime_error);
    }
    
    SECTION("Get all elements") {
        Element::initialize();
        
        auto all_elements = Element::get_all_elements();
        REQUIRE(all_elements.size() == Element::count());
        
        // Check that we have some common elements
        bool has_hydrogen = false;
        bool has_carbon = false;
        bool has_oxygen = false;
        
        for (const auto& element : all_elements) {
            if (element.symbol == "H") has_hydrogen = true;
            if (element.symbol == "C") has_carbon = true;
            if (element.symbol == "O") has_oxygen = true;
        }
        
        REQUIRE(has_hydrogen == true);
        REQUIRE(has_carbon == true);
        REQUIRE(has_oxygen == true);
    }
}
