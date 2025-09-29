#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <xtensor/containers/xarray.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("Param system basic functionality", "[core][param]") {
    SECTION("ParamValue basic operations") {
        ParamValue pv;
        
        // Test assignment and get
        pv = 42;
        REQUIRE(pv.get<int>() == 42);
        
        pv = 3.14;
        REQUIRE(pv.get<double>() == 3.14);
        
        pv = std::string("hello");
        REQUIRE(pv.get<std::string>() == "hello");
        
        // Test has_value
        REQUIRE(pv.has_value());
        
        ParamValue empty;
        REQUIRE_FALSE(empty.has_value());
    }
    
    SECTION("ParamList basic operations") {
        ParamList plist;
        
        // Test assignment via operator[]
        plist[0] = 42;
        plist[1] = 3.14;
        plist[2] = std::string("test");
        
        // Test implicit conversion
        int a = plist[0];
        double b = plist[1];
        std::string c = plist[2];
        
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        REQUIRE(c == "test");
        
        // Test ref access
        int& ref_a = plist.ref<int>(0);
        ref_a = 100;
        REQUIRE(plist.get<int>(0) == 100);
        
        // Test auto-expansion
        plist[10] = 999;
        REQUIRE(plist.size() == 11);
        REQUIRE(plist.get<int>(10) == 999);
    }
    
    SECTION("ParamMap basic operations") {
        ParamMap pmap;
        
        // Test assignment via operator[]
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
        
        // Test ref access
        int& ref_a = pmap.ref<int>("int_val");
        ref_a = 100;
        REQUIRE(pmap.get<int>("int_val") == 100);
        
        // Test contains
        REQUIRE(pmap.contains("int_val"));
        REQUIRE_FALSE(pmap.contains("nonexistent"));
    }
    
    SECTION("ParamContainer unified access") {
        ParamList list;
        list[0] = 42;
        list[1] = 3.14;
        
        ParamMap map;
        map["name"] = std::string("test");
        map["flag"] = true;
        
        ParamContainer container(list, map);
        
        // Test list access
        int a = container[0];
        double b = container[1];
        REQUIRE(a == 42);
        REQUIRE(b == 3.14);
        
        // Test map access
        std::string name = container["name"];
        bool flag = container["flag"];
        REQUIRE(name == "test");
        REQUIRE(flag == true);
        
        // Test direct access methods
        REQUIRE(container.get<int>(0) == 42);
        REQUIRE(container.get<std::string>("name") == "test");
        
        // Test utility methods
        REQUIRE(container.has("name"));
        REQUIRE_FALSE(container.has("nonexistent"));
        REQUIRE(container.list_size() == 2);
        REQUIRE(container.map_size() == 2);
    }
    
    SECTION("ValueRef proxy functionality") {
        ParamList plist;
        plist[0] = 42;
        
        // Test ValueRef assignment
        ValueRef ref = plist[0];
        ref = 100;
        REQUIRE(plist.get<int>(0) == 100);
        
        // Test ValueRef implicit conversion
        int value = ref;
        REQUIRE(value == 100);
        
        // Test ValueRef ref() method
        int& direct_ref = ref.ref<int>();
        direct_ref = 200;
        REQUIRE(plist.get<int>(0) == 200);
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
        
        ParamMap pmap;
        pmap["name"] = std::string("PEG");
        std::string s = pmap["name"];
        REQUIRE(s == "PEG");
    }
}
