#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/element.hpp>

using namespace molcpp;

TEST_CASE("Element basic functionality", "[core][element]") {
    SECTION("Element creation and properties") {
        // Initialize the element database
        Element::initialize();
        
        // Test basic element creation
        auto carbon = Element::create("C");
        
        REQUIRE(carbon.symbol == "C");
        REQUIRE(carbon.number == 6);
        REQUIRE(std::abs(carbon.mass - 12.01078) < 1e-6);
    }
    
    SECTION("Element comparison") {
        Element::initialize();
        
        auto carbon1 = Element::create("C");
        auto carbon2 = Element::create(6);  // Test atomic number lookup
        auto hydrogen = Element::create("H");
        
        REQUIRE(carbon1.symbol == carbon2.symbol);
        REQUIRE(carbon1.number == carbon2.number);
        REQUIRE(carbon1.symbol != hydrogen.symbol);
        REQUIRE(carbon1.number != hydrogen.number);
    }
    
    SECTION("Element factory methods") {
        Element::initialize();
        
        // Test get_atomic_number method
        REQUIRE(Element::get_atomic_number("C") == 6);
        REQUIRE(Element::get_atomic_number("H") == 1);
        
        // Test get_symbols method
        std::vector<std::string> identifiers = {"C", "H", "O"};
        auto symbols = Element::get_symbols(identifiers);
        REQUIRE(symbols.size() == 3);
        REQUIRE(symbols[0] == "C");
        REQUIRE(symbols[1] == "H");
        REQUIRE(symbols[2] == "O");
        
        // Test exists method
        REQUIRE(Element::exists("C"));
        REQUIRE(Element::exists(6));
        REQUIRE(!Element::exists("Xx"));  // Non-existent element
    }
}
