#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/forcefield.hpp>
#include <string>

using namespace molcpp;

TEST_CASE("ParamContainer functionality", "[core][param][paramcontainer]") {
    SECTION("Default construction") {
        ParamContainer container;
        
        REQUIRE(container.empty());
        REQUIRE(container.list_size() == 0);
        REQUIRE(container.map_size() == 0);
    }
    
    SECTION("Construction with ParamList") {
        ParamList list;
        list[0] = 42;
        list[1] = 3.14;
        
        ParamContainer container(list);
        
        REQUIRE(container.list_size() == 2);
        REQUIRE(container.map_size() == 0);
        REQUIRE_FALSE(container.empty());
    }
    
    SECTION("Construction with ParamMap") {
        ParamMap map;
        map["key1"] = 42;
        map["key2"] = 3.14;
        
        ParamContainer container(map);
        
        REQUIRE(container.list_size() == 0);
        REQUIRE(container.map_size() == 2);
        REQUIRE_FALSE(container.empty());
    }
    
    SECTION("Construction with both") {
        ParamList list;
        list[0] = 42;
        list[1] = 3.14;
        
        ParamMap map;
        map["key1"] = "hello";
        map["key2"] = "world";
        
        ParamContainer container(list, map);
        
        REQUIRE(container.list_size() == 2);
        REQUIRE(container.map_size() == 2);
        REQUIRE_FALSE(container.empty());
    }
    
    SECTION("Unified access via operator[]") {
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
    }
    
    SECTION("Direct access methods") {
        ParamList list;
        list[0] = 42;
        list[1] = 3.14;
        
        ParamMap map;
        map["int_val"] = 100;
        map["string_val"] = std::string("hello");
        
        ParamContainer container(list, map);
        
        // Test list access methods
        REQUIRE(container.get<int>(0) == 42);
        REQUIRE(container.get<double>(1) == 3.14);
        
        int& list_ref = container.ref<int>(0);
        list_ref = 200;
        REQUIRE(container.get<int>(0) == 200);
        
        // Test map access methods
        REQUIRE(container.get<int>("int_val") == 100);
        REQUIRE(container.get<std::string>("string_val") == "hello");
        
        int& map_ref = container.ref<int>("int_val");
        map_ref = 300;
        REQUIRE(container.get<int>("int_val") == 300);
    }
    
    SECTION("Utility methods") {
        ParamList list;
        list[0] = 42;
        
        ParamMap map;
        map["key1"] = 3.14;
        map["key2"] = std::string("test");
        
        ParamContainer container(list, map);
        
        // Test has() method
        REQUIRE(container.has("key1"));
        REQUIRE(container.has("key2"));
        REQUIRE_FALSE(container.has("nonexistent"));
        
        // Test size methods
        REQUIRE(container.list_size() == 1);
        REQUIRE(container.map_size() == 2);
        REQUIRE_FALSE(container.empty());
    }
    
    SECTION("Container access") {
        ParamList list;
        list[0] = 42;
        
        ParamMap map;
        map["key1"] = 3.14;
        
        ParamContainer container(list, map);
        
        // Test list() access
        auto& list_ref = container.list();
        REQUIRE(list_ref.size() == 1);
        REQUIRE(list_ref.get<int>(0) == 42);
        
        const auto& const_list_ref = container.list();
        REQUIRE(const_list_ref.size() == 1);
        
        // Test map() access
        auto& map_ref = container.map();
        REQUIRE(map_ref.size() == 1);
        REQUIRE(map_ref.get<double>("key1") == 3.14);
        
        const auto& const_map_ref = container.map();
        REQUIRE(const_map_ref.size() == 1);
    }
    
    SECTION("Clear operation") {
        ParamList list;
        list[0] = 42;
        
        ParamMap map;
        map["key1"] = 3.14;
        
        ParamContainer container(list, map);
        
        REQUIRE_FALSE(container.empty());
        
        container.clear();
        
        REQUIRE(container.empty());
        REQUIRE(container.list_size() == 0);
        REQUIRE(container.map_size() == 0);
    }
}
