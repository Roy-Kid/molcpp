#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("ParamMap functionality", "[core][param][parammap]") {
    SECTION("Basic operations") {
        ParamMap pmap;
        
        // Test initial state
        REQUIRE(pmap.empty());
        REQUIRE(pmap.size() == 0);
        
        // Test assignment via operator[]
        pmap["int_val"] = 42;
        pmap["double_val"] = 3.14;
        pmap["string_val"] = std::string("test");
        
        // Test size after assignment
        REQUIRE(pmap.size() == 3);
        REQUIRE_FALSE(pmap.empty());
    }
    
    SECTION("Implicit type conversion") {
        ParamMap pmap;
        pmap["int_val"] = 42;
        pmap["double_val"] = 3.14;
        pmap["string_val"] = std::string("hello");
        
        // Test implicit conversion
        int a = pmap["int_val"];
        double b = pmap["double_val"];
        std::string c = pmap["string_val"];
        
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        REQUIRE(c == "hello");
    }
    
    SECTION("Reference access") {
        ParamMap pmap;
        pmap["int_val"] = 42;
        
        // Test ref() method
        int& ref = pmap.ref<int>("int_val");
        ref = 100;
        REQUIRE(pmap.get<int>("int_val") == 100);
        
        // Test const ref
        const ParamMap& const_pmap = pmap;
        const int& const_ref = const_pmap.ref<int>("int_val");
        REQUIRE(const_ref == 100);
    }
    
    SECTION("Key existence checks") {
        ParamMap pmap;
        pmap["key1"] = 42;
        pmap["key2"] = 3.14;
        
        // Test contains
        REQUIRE(pmap.contains("key1"));
        REQUIRE(pmap.contains("key2"));
        REQUIRE_FALSE(pmap.contains("nonexistent"));
    }
    
    SECTION("Direct access methods") {
        ParamMap pmap;
        pmap["int_val"] = 42;
        pmap["double_val"] = 3.14;
        
        // Test get() method
        int a = pmap.get<int>("int_val");
        double b = pmap.get<double>("double_val");
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        
        // Test ref() method
        int& ref = pmap.ref<int>("int_val");
        ref = 100;
        REQUIRE(pmap.get<int>("int_val") == 100);
    }
    
    SECTION("Container operations") {
        ParamMap pmap;
        pmap["key1"] = 42;
        pmap["key2"] = 3.14;
        
        // Test clear
        pmap.clear();
        REQUIRE(pmap.empty());
        REQUIRE(pmap.size() == 0);
    }
    
    SECTION("Data access") {
        ParamMap pmap;
        pmap["key1"] = 42;
        pmap["key2"] = 3.14;
        
        // Test direct access to underlying map
        auto& data = pmap.data();
        REQUIRE(data.size() == 2);
        
        const auto& const_data = pmap.data();
        REQUIRE(const_data.size() == 2);
    }
    
    SECTION("Iterator support") {
        ParamMap pmap;
        pmap["key1"] = 42;
        pmap["key2"] = 3.14;
        
        // Test iteration
        int count = 0;
        for (const auto& pair : pmap) {
            count++;
            REQUIRE((pair.first == "key1" || pair.first == "key2"));
        }
        REQUIRE(count == 2);
    }
    
    SECTION("Example usage from requirements") {
        // Test the exact example from requirements
        ParamMap pmap;
        pmap["name"] = std::string("PEG");
        std::string s = pmap["name"];
        REQUIRE(s == "PEG");
    }
}
