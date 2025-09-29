#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("ValueRef proxy functionality", "[core][param][valueref]") {
    SECTION("Basic assignment and conversion") {
        ParamValue pv;
        ValueRef ref(&pv);
        
        // Test assignment
        ref = 42;
        REQUIRE(pv.get<int>() == 42);
        
        // Test implicit conversion
        int value = ref;
        REQUIRE(value == 42);
        
        // Test different types
        ref = 3.14;
        double dvalue = ref;
        REQUIRE(dvalue == 3.14);
        
        ref = std::string("hello");
        std::string svalue = ref;
        REQUIRE(svalue == "hello");
    }
    
    SECTION("Reference access") {
        ParamValue pv(42);
        ValueRef ref(&pv);
        
        // Test ref() method
        int& direct_ref = ref.ref<int>();
        direct_ref = 100;
        REQUIRE(pv.get<int>() == 100);
        
        // Test const ref
        const ValueRef const_ref(&pv);
        const int& const_direct_ref = const_ref.ref<int>();
        REQUIRE(const_direct_ref == 100);
    }
    
    SECTION("Has value check") {
        ParamValue pv;
        ValueRef ref(&pv);
        
        REQUIRE_FALSE(ref.has_value());
        
        ref = 42;
        REQUIRE(ref.has_value());
    }
    
    SECTION("Null pointer handling") {
        ValueRef null_ref(nullptr);
        
        // Should not crash (though behavior is undefined)
        null_ref = 42;
        REQUIRE_FALSE(null_ref.has_value());
    }
}
