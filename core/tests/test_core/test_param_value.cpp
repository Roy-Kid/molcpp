#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("ParamValue basic functionality", "[core][param][paramvalue]") {
    SECTION("Default construction") {
        ParamValue pv;
        REQUIRE_FALSE(pv.has_value());
    }
    
    SECTION("Value construction and assignment") {
        // Test int
        ParamValue pv1(42);
        REQUIRE(pv1.has_value());
        REQUIRE(pv1.get<int>() == 42);
        
        // Test double
        ParamValue pv2(3.14);
        REQUIRE(pv2.has_value());
        REQUIRE(pv2.get<double>() == 3.14);
        
        // Test string
        ParamValue pv3(std::string("hello"));
        REQUIRE(pv3.has_value());
        REQUIRE(pv3.get<std::string>() == "hello");
        
        // Test assignment
        pv1 = 100;
        REQUIRE(pv1.get<int>() == 100);
        
        pv2 = 2.718;
        REQUIRE(pv2.get<double>() == 2.718);
    }
    
    SECTION("Template assignment") {
        ParamValue pv;
        
        // Test set method
        pv.set(42);
        REQUIRE(pv.get<int>() == 42);
        
        pv.set(3.14);
        REQUIRE(pv.get<double>() == 3.14);
        
        pv.set(std::string("world"));
        REQUIRE(pv.get<std::string>() == "world");
    }
    
    SECTION("Type safety") {
        ParamValue pv(42);
        
        // These should work (assuming user knows correct type)
        int i = pv.get<int>();
        REQUIRE(i == 42);
        
        // Test const access
        const ParamValue& const_pv = pv;
        int ci = const_pv.get<int>();
        REQUIRE(ci == 42);
    }
}
