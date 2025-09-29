#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <xtensor/containers/xarray.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("ParamList functionality", "[core][param][paramlist]") {
    SECTION("Basic operations") {
        ParamList plist;
        
        // Test initial state
        REQUIRE(plist.empty());
        REQUIRE(plist.size() == 0);
        
        // Test assignment via operator[]
        plist[0] = 42;
        plist[1] = 3.14;
        plist[2] = std::string("test");
        
        // Test size after assignment
        REQUIRE(plist.size() == 3);
        REQUIRE_FALSE(plist.empty());
    }
    
    SECTION("Implicit type conversion") {
        ParamList plist;
        plist[0] = 42;
        plist[1] = 3.14;
        plist[2] = std::string("hello");
        
        // Test implicit conversion
        int a = plist[0];
        double b = plist[1];
        std::string c = plist[2];
        
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        REQUIRE(c == "hello");
    }
    
    SECTION("Reference access") {
        ParamList plist;
        plist[0] = 42;
        
        // Test ref() method
        int& ref = plist.ref<int>(0);
        ref = 100;
        REQUIRE(plist.get<int>(0) == 100);
        
        // Test const ref
        const ParamList& const_plist = plist;
        const int& const_ref = const_plist.ref<int>(0);
        REQUIRE(const_ref == 100);
    }
    
    SECTION("Auto-expansion") {
        ParamList plist;
        
        // Test auto-expansion when writing to out-of-range index
        plist[10] = 999;
        REQUIRE(plist.size() == 11);
        REQUIRE(plist.get<int>(10) == 999);
        
        // Test that intermediate elements are default-constructed
        plist[5] = 555;
        REQUIRE(plist.size() == 11);
        REQUIRE(plist.get<int>(5) == 555);
    }
    
    SECTION("Direct access methods") {
        ParamList plist;
        plist[0] = 42;
        plist[1] = 3.14;
        
        // Test get() method
        int a = plist.get<int>(0);
        double b = plist.get<double>(1); // Get as double
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        
        // Test ref() method with auto-expansion
        plist[5] = 555;  // First assign a value
        int& ref = plist.ref<int>(5);
        ref = 666;  // Then modify via reference
        REQUIRE(plist.size() == 6);
        REQUIRE(plist.get<int>(5) == 666);
    }
    
    SECTION("Container operations") {
        ParamList plist;
        plist[0] = 42;
        plist[1] = 3.14;
        
        // Test resize
        plist.resize(10);
        REQUIRE(plist.size() == 10);
        
        // Test clear
        plist.clear();
        REQUIRE(plist.empty());
        REQUIRE(plist.size() == 0);
    }
    
    SECTION("Data access") {
        ParamList plist;
        plist[0] = 42;
        plist[1] = 3.14;
        
        // Test direct access to underlying vector
        auto& data = plist.data();
        REQUIRE(data.size() == 2);
        
        const auto& const_data = plist.data();
        REQUIRE(const_data.size() == 2);
    }
    
    SECTION("Example usage from requirements") {
        // Test the exact example from requirements
        ParamList plist;
        plist[0] = 42;
        plist[1] = 3.14;
        int a = plist[0];      // implicit operator T()
        double b = plist[1];   // works too
        
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        
        // Test with xtensor (if available)
        xt::xarray<int> arr = {1, 2, 3, 4, 5};
        plist[2] = arr;
        auto& arr_ref = plist[2].ref<xt::xarray<int>>(); // zero-copy reference
        REQUIRE(arr_ref.size() == 5);
        REQUIRE(arr_ref(0) == 1);
    }
}
