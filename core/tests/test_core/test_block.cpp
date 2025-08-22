#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/core/block.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/views/xview.hpp>
#include <xtensor/generators/xrandom.hpp>

using namespace molcpp;
using namespace Catch;

TEST_CASE("Block basic operations", "[block]") {
    Block block;
    
    SECTION("Empty state") {
        REQUIRE(block.empty());
        REQUIRE(block.size() == 0);
        REQUIRE(block.get_size() == 0);
    }
    
    SECTION("Simple operations") {
        REQUIRE(block.empty());
        REQUIRE(block.size() == 0);
        
        // Test with simple data
        xt::xarray<int> ids = {1, 2, 3};
        xt::xarray<double> coords = {1.1, 2.2, 3.3};
        
        block.set("ids", ids);
        block.set("coords", coords);
        
        REQUIRE(block.size() == 2);
        REQUIRE(block.contains("ids"));
        REQUIRE(block.contains("coords"));
        
        // Test natural syntax with automatic type conversion
        xt::xarray<int> retrieved_ids = block["ids"];
        xt::xarray<double> retrieved_coords = block["coords"];
        
        REQUIRE(retrieved_ids(0) == 1);
        REQUIRE(retrieved_coords(0) == Approx(1.1));
    }
    
    SECTION("Set and get operations") {
        xt::xarray<int> data = {1, 2, 3, 4, 5};
        block.set("data", data);
        
        REQUIRE(block.contains("data"));
        REQUIRE(block.size() == 1);
        
        xt::xarray<int> retrieved = block["data"];
        REQUIRE(retrieved.size() == 5);
        REQUIRE(retrieved(0) == 1);
        REQUIRE(retrieved(4) == 5);
    }
    
    SECTION("Multiple data types") {
        xt::xarray<int> int_data = {1, 2, 3};
        xt::xarray<double> double_data = {1.1, 2.2, 3.3};
        xt::xarray<float> float_data = {1.0f, 2.0f, 3.0f};
        
        block.set("int_data", int_data);
        block.set("double_data", double_data);
        block.set("float_data", float_data);
        
        REQUIRE(block.size() == 3);
        REQUIRE(block.contains("int_data"));
        REQUIRE(block.contains("double_data"));
        REQUIRE(block.contains("float_data"));
        
        xt::xarray<int> retrieved_int = block["int_data"];
        xt::xarray<double> retrieved_double = block["double_data"];
        xt::xarray<float> retrieved_float = block["float_data"];
        
        REQUIRE(retrieved_int(0) == 1);
        REQUIRE(retrieved_double(1) == Approx(2.2));
        REQUIRE(retrieved_float(2) == Approx(3.0f));
    }
    
    SECTION("Const access") {
        xt::xarray<int> data = {1, 2, 3};
        block.set("data", data);
        
        const Block& const_block = block;
        xt::xarray<int> retrieved = const_block["data"];
        REQUIRE(retrieved(0) == 1);
        REQUIRE(retrieved(1) == 2);
        REQUIRE(retrieved(2) == 3);
    }
    
    SECTION("Move operations") {
        xt::xarray<int> data = {1, 2, 3};
        block.set("data", std::move(data));
        
        REQUIRE(block.contains("data"));
        xt::xarray<int> retrieved = block["data"];
        REQUIRE(retrieved(0) == 1);
    }
    
    SECTION("Erase operations") {
        xt::xarray<int> data = {1, 2, 3};
        block.set("data", data);
        
        REQUIRE(block.contains("data"));
        block.erase("data");
        REQUIRE_FALSE(block.contains("data"));
        REQUIRE(block.size() == 0);
    }
    
    SECTION("Clear operations") {
        xt::xarray<int> data1 = {1, 2, 3};
        xt::xarray<double> data2 = {1.1, 2.2, 3.3};
        
        block.set("data1", data1);
        block.set("data2", data2);
        
        REQUIRE(block.size() == 2);
        block.clear();
        REQUIRE(block.size() == 0);
        REQUIRE(block.empty());
    }
    
    SECTION("Get span operations") {
        xt::xarray<int> data = {1, 2, 3, 4, 5};
        block.set("data", data);
        
        auto span = block.get_span<int>("data");
        REQUIRE(span.size() == 5);
        REQUIRE(span[0] == 1);
        REQUIRE(span[4] == 5);
        
        const auto& const_span = block.get_span<int>("data");
        REQUIRE(const_span.size() == 5);
        REQUIRE(const_span[0] == 1);
    }
    
    SECTION("Large data handling") {
        const size_t size = 1000;
        xt::xarray<int> large_int = xt::zeros<int>({size});
        xt::xarray<double> large_double = xt::zeros<double>({size});
        for (size_t i = 0; i < size; ++i) {
            large_int[i] = i;
            large_double[i] = i * 0.1;
        }
        
        block.set("large_int", large_int);
        block.set("large_double", large_double);
        
        REQUIRE(block.size() == 2);
        REQUIRE(block.get_size() == 2); // 2 variables: large_int and large_double
        
        // Test random access
        for (size_t i = 0; i < size; i += 100) {
            xt::xarray<int> large_int_data = block["large_int"];
            xt::xarray<double> large_double_data = block["large_double"];
            REQUIRE(large_int_data(i) == static_cast<int>(i));
            REQUIRE(large_double_data(i) == Approx(static_cast<double>(i) * 0.1));
        }
    }
    
    SECTION("Error handling") {
        // Test that accessing non-existent keys throws
        REQUIRE_THROWS_AS(block.at("nonexistent"), std::out_of_range);
        REQUIRE_THROWS_AS(block.get_span<int>("nonexistent"), std::out_of_range);
        
        const Block& const_block = block;
        REQUIRE_THROWS_AS(const_block.at("nonexistent"), std::out_of_range);
        REQUIRE_THROWS_AS(const_block.get_span<int>("nonexistent"), std::out_of_range);
    }
    
    SECTION("Copy and move semantics") {
        xt::xarray<int> data = {1, 2, 3};
        block.set("data", data);
        
        // Copy constructor
        Block block_copy(block);
        REQUIRE(block_copy.size() == block.size());
        REQUIRE(block_copy.contains("data"));
        xt::xarray<int> copy_data = block_copy["data"];
        REQUIRE(copy_data(0) == 1);
        
        // Copy assignment
        Block block_copy2;
        block_copy2 = block;
        REQUIRE(block_copy2.size() == block.size());
        REQUIRE(block_copy2.contains("data"));
        
        // Move constructor
        Block block_move(std::move(block_copy));
        REQUIRE(block_move.size() == 1);
        REQUIRE(block_move.contains("data"));
        
        // Move assignment
        Block block_move2;
        block_move2 = std::move(block_move);
        REQUIRE(block_move2.size() == 1);
        REQUIRE(block_move2.contains("data"));
    }
    
    SECTION("Type safety") {
        xt::xarray<int> int_data = {1, 2, 3};
        xt::xarray<double> double_data = {1.1, 2.2, 3.3};
        
        block.set("int_data", int_data);
        block.set("double_data", double_data);
        
        // Should compile and work correctly with automatic type deduction
        xt::xarray<int> int_result = block["int_data"];
        xt::xarray<double> double_result = block["double_data"];
        
        REQUIRE(int_result(0) == 1);
        REQUIRE(double_result(0) == Approx(1.1));
        
        // Test auto deduction - auto will be BlockProxy, need explicit conversion
        auto auto_int = block["int_data"];
        auto auto_double = block["double_data"];
        
        // Convert to actual xtensor types
        xt::xarray<int> int_array = auto_int;
        xt::xarray<double> double_array = auto_double;
        
        // Should be able to use as xtensor arrays
        REQUIRE(int_array(0) == 1);
        REQUIRE(double_array(0) == Approx(1.1));
    }
    
    SECTION("Natural syntax: xt::xarray<T> = block[\"key\"]") {
        // This is the EXACT syntax you requested!
        xt::xarray<int> ids = {1, 2, 3, 4, 5};
        xt::xarray<double> coords = {1.1, 2.2, 3.3, 4.4, 5.5};
        xt::xarray<float> weights = {0.1f, 0.2f, 0.3f};
        
        block.set("ids", ids);
        block.set("coords", coords);
        block.set("weights", weights);
        
        // Test the natural syntax with explicit type specification
        xt::xarray<int> retrieved_ids = block["ids"];
        xt::xarray<double> retrieved_coords = block["coords"];
        xt::xarray<float> retrieved_weights = block["weights"];
        
        // Verify correctness
        REQUIRE(retrieved_ids.size() == 5);
        REQUIRE(retrieved_coords.size() == 5);
        REQUIRE(retrieved_weights.size() == 3);
        
        REQUIRE(retrieved_ids(0) == 1);
        REQUIRE(retrieved_ids(4) == 5);
        REQUIRE(retrieved_coords(0) == Approx(1.1));
        REQUIRE(retrieved_coords(4) == Approx(5.5));
        REQUIRE(retrieved_weights(0) == Approx(0.1f));
        REQUIRE(retrieved_weights(2) == Approx(0.3f));
        
        // Test that it creates actual copies (not references)
        retrieved_ids(0) = 999;
        xt::xarray<int> original_ids = block["ids"];
        REQUIRE(original_ids(0) == 1); // Original should be unchanged
    }
    
    SECTION("Python-like methods: get<T>, set, contains") {
        xt::xarray<int> data = {10, 20, 30, 40, 50};
        xt::xarray<double> scores = {0.95, 0.87, 0.92};
        
        // Test set method
        block.set("data", data);
        block.set("scores", scores);
        
        // Test contains method
        REQUIRE(block.contains("data") == true);
        REQUIRE(block.contains("scores") == true);
        REQUIRE(block.contains("nonexistent") == false);
        
        // Test get<T> method (Python dict.get equivalent)
        auto& retrieved_data = block.get<int>("data");
        auto& retrieved_scores = block.get<double>("scores");
        
        REQUIRE(retrieved_data.size() == 5);
        REQUIRE(retrieved_scores.size() == 3);
        REQUIRE(retrieved_data(0) == 10);
        REQUIRE(retrieved_scores(0) == Approx(0.95));
        
        // Test const get<T>
        const Block& const_block = block;
        const auto& const_data = const_block.get<int>("data");
        REQUIRE(const_data(0) == 10);
        
        // Test error handling with get<T>
        REQUIRE_THROWS_AS(block.get<int>("nonexistent"), std::out_of_range);
        REQUIRE_THROWS_AS(block.get<double>("data"), std::runtime_error); // Type mismatch
    }
    
    SECTION("Auto type deduction with block[\"key\"]") {
        xt::xarray<int> data = {10, 20, 30};
        block.set("data", data);
        
        // Test auto deduction - auto will be BlockProxy, need explicit conversion
        auto auto_data = block["data"];
        
        // Convert to actual xtensor type
        xt::xarray<int> data_array = auto_data;
        
        // Should work as xtensor
        REQUIRE(data_array.size() == 3);
        REQUIRE(data_array(0) == 10);
        REQUIRE(data_array(1) == 20);
        REQUIRE(data_array(2) == 30);
        
        // Should be able to do xtensor operations
        auto sum = xt::sum(data_array);
        REQUIRE(sum() == 60);
    }
}
