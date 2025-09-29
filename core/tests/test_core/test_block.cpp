#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/block.hpp>
#include <xtensor/containers/xarray.hpp>

using namespace molcpp;

TEST_CASE("Block basic functionality", "[core][block]") {
    SECTION("Block creation and basic operations") {
        Block block;
        
        // Test basic block properties
        REQUIRE(block.empty());
        REQUIRE(block.size() == 0);
        REQUIRE(!block.contains("test"));
    }
    
    SECTION("Block set and get operations") {
        Block block;
        
        // Create some test data
        xt::xarray<int> int_data = {1, 2, 3, 4, 5};
        xt::xarray<double> double_data = {1.1, 2.2, 3.3, 4.4, 5.5};
        xt::xarray<float> float_data = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
        
        // Test set operations
        block.set("integers", int_data);
        block.set("doubles", double_data);
        block.set("floats", float_data);
        
        // Test basic properties
        REQUIRE(block.size() == 3);
        REQUIRE(!block.empty());
        REQUIRE(block.contains("integers"));
        REQUIRE(block.contains("doubles"));
        REQUIRE(block.contains("floats"));
        REQUIRE(!block.contains("nonexistent"));
        
        // Test get operations
        xt::xarray<int>& retrieved_int = block.get<int>("integers");
        xt::xarray<double>& retrieved_double = block.get<double>("doubles");
        xt::xarray<float>& retrieved_float = block.get<float>("floats");
        
        REQUIRE(retrieved_int.size() == 5);
        REQUIRE(retrieved_double.size() == 5);
        REQUIRE(retrieved_float.size() == 5);
        
        REQUIRE(retrieved_int(0) == 1);
        REQUIRE(retrieved_double(0) == 1.1);
        REQUIRE(retrieved_float(0) == 1.1f);
    }
    
    SECTION("Block proxy operations") {
        Block block;
        
        // Create test data
        xt::xarray<int> int_data = {10, 20, 30};
        xt::xarray<double> double_data = {1.5, 2.5, 3.5};
        
        // Test proxy assignment
        block["integers"] = int_data;
        block["doubles"] = double_data;
        
        // CORRECT: Must use explicit type declaration when getting data from proxy
        xt::xarray<int> int_data_from_proxy = block["integers"];
        xt::xarray<double> double_data_from_proxy = block["doubles"];
        
        // CORRECT: Direct reference assignment (simpler)
        xt::xarray<int>& int_ref = block["integers"];
        xt::xarray<double>& double_ref = block["doubles"];
        
        // Test both approaches
        REQUIRE(int_data_from_proxy.size() == 3);
        REQUIRE(double_data_from_proxy.size() == 3);
        REQUIRE(int_data_from_proxy(0) == 10);
        REQUIRE(double_data_from_proxy(0) == 1.5);
        
        REQUIRE(int_ref.size() == 3);
        REQUIRE(double_ref.size() == 3);
        REQUIRE(int_ref(0) == 10);
        REQUIRE(double_ref(0) == 1.5);
    }
    
    SECTION("Block proxy correct usage patterns") {
        Block block;
        
        xt::xarray<float> float_data = {1.1f, 2.2f, 3.3f};
        block["floats"] = float_data;
        
        // Pattern 1: Direct assignment to typed variable (recommended)
        xt::xarray<float> retrieved_float = block["floats"];
        REQUIRE(retrieved_float.size() == 3);
        REQUIRE(retrieved_float(0) == 1.1f);
        
        // Pattern 2: Direct reference assignment (simplest for references)
        xt::xarray<float>& float_ref = block["floats"];
        REQUIRE(float_ref.size() == 3);
        REQUIRE(float_ref(0) == 1.1f);
        
        // Pattern 3: Using get<T> method (alternative for references)
        xt::xarray<float>& float_get = block.get<float>("floats");
        REQUIRE(float_get.size() == 3);
        REQUIRE(float_get(0) == 1.1f);
        
        // WRONG: auto will give you BlockProxy, not xt::xarray
        // auto wrong_usage = block["floats"];  // This is BlockProxy, not xt::xarray<float>
    }
    
    SECTION("Block type checking") {
        Block block;
        
        xt::xarray<int> int_data = {1, 2, 3};
        xt::xarray<double> double_data = {1.1, 2.2, 3.3};
        
        block.set("integers", int_data);
        block.set("doubles", double_data);
        
        // Test type checking
        REQUIRE(block.has_type<int>("integers"));
        REQUIRE(block.has_type<double>("doubles"));
        REQUIRE(!block.has_type<float>("integers"));
        REQUIRE(!block.has_type<int>("doubles"));
        
        // Test type information
        REQUIRE(block.get_type("integers") == "int");
        REQUIRE(block.get_type("doubles") == "double");
        
        auto types = block.get_types();
        REQUIRE(types.size() == 2);
        REQUIRE(types["integers"] == "int");
        REQUIRE(types["doubles"] == "double");
    }
    
    SECTION("Block iteration and range operations") {
        Block block;
        
        xt::xarray<int> data1 = {1, 2, 3};
        xt::xarray<double> data2 = {1.1, 2.2, 3.3};
        
        block.set("data1", data1);
        block.set("data2", data2);
        
        // Test iteration
        int count = 0;
        for (const auto& [key, value] : block) {
            count++;
            REQUIRE((key == "data1" || key == "data2"));
        }
        REQUIRE(count == 2);
        
        // Test range operations
        auto keys = block.keys();
        std::vector<std::string> key_vec(keys.begin(), keys.end());
        REQUIRE(key_vec.size() == 2);
        REQUIRE(std::find(key_vec.begin(), key_vec.end(), "data1") != key_vec.end());
        REQUIRE(std::find(key_vec.begin(), key_vec.end(), "data2") != key_vec.end());
    }
    
    SECTION("Block error handling") {
        Block block;
        
        // Test accessing non-existent key
        REQUIRE_THROWS_AS(block.at("nonexistent"), std::out_of_range);
        REQUIRE_THROWS_AS(block.get<int>("nonexistent"), std::out_of_range);
        
        // Test type mismatch
        xt::xarray<int> int_data = {1, 2, 3};
        block.set("data", int_data);
        
        REQUIRE_THROWS_AS(block.get<double>("data"), std::runtime_error);
    }
}
