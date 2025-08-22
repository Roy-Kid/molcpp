#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/core/frame.hpp"
#include "molcpp/core/block.hpp"
#include <xtensor/containers/xarray.hpp>
// xtensor/random.hpp not available, using arange instead

using namespace molcpp;
using namespace Catch;

TEST_CASE("Frame basic operations", "[frame]") {
    Frame frame;
    
    SECTION("Empty state") {
        REQUIRE(frame.empty());
        REQUIRE(frame.get_block_count() == 0);
        REQUIRE(frame.n_blocks() == 0);
        REQUIRE(frame.n_variables() == 0);
    }
    
    SECTION("Simple operations") {
        REQUIRE(frame.empty());
        REQUIRE(frame.get_block_count() == 0);
        
        // Create a simple block
        Block atoms;
        xt::xarray<int> ids = {1, 2, 3};
        atoms.set("id", ids);
        
        frame.set_block("atoms", atoms);
        
        REQUIRE(frame.get_block_count() == 1);
        REQUIRE(frame.contains_block("atoms"));
        REQUIRE(frame.n_blocks() == 1);
        
        // Test variable access across blocks
        auto& atom_ids = frame.operator()<int>("atoms", "id");
        REQUIRE(atom_ids(0) == 1);
        REQUIRE(atom_ids(1) == 2);
        REQUIRE(atom_ids(2) == 3);
    }
    
    SECTION("Metadata operations") {
        frame.set_metadata("title", std::string("Test Frame"));
        frame.set_metadata("version", 1.0);
        
        REQUIRE(frame.has_metadata("title"));
        REQUIRE(frame.has_metadata("version"));
        REQUIRE(frame.contains_metadata("title"));
        
        REQUIRE(frame.get_metadata<std::string>("title") == "Test Frame");
        REQUIRE(frame.get_metadata<double>("version") == Approx(1.0));
    }
    
    SECTION("Block management") {
        Block atoms, bonds;
        xt::xarray<int> atom_ids = {1, 2, 3};
        xt::xarray<int> bond_ids = {1, 2};
        
        atoms.set("id", atom_ids);
        bonds.set("id", bond_ids);
        
        frame.set_block("atoms", atoms);
        frame.set_block("bonds", bonds);
        
        REQUIRE(frame.get_block_count() == 2);
        REQUIRE(frame.contains_block("atoms"));
        REQUIRE(frame.contains_block("bonds"));
        REQUIRE_FALSE(frame.contains_block("nonexistent"));
        
        // Test block access
        const auto& retrieved_atoms = frame["atoms"];
        const auto& retrieved_bonds = frame["bonds"];
        
        REQUIRE(retrieved_atoms.size() == 1);
        REQUIRE(retrieved_bonds.size() == 1);
        
        // Test block removal
        frame.remove_block("bonds");
        REQUIRE(frame.get_block_count() == 1);
        REQUIRE_FALSE(frame.contains_block("bonds"));
    }
    
    SECTION("Variable access") {
        Block atoms;
        xt::xarray<int> ids = {1, 2, 3};
        xt::xarray<double> coords = {1.1, 2.2, 3.3};
        
        atoms.set("id", ids);
        atoms.set("coords", coords);
        frame.set_block("atoms", atoms);
        
        // Test operator() access
        auto& atom_ids = frame.operator()<int>("atoms", "id");
        auto& atom_coords = frame.operator()<double>("atoms", "coords");
        
        REQUIRE(atom_ids(0) == 1);
        REQUIRE(atom_coords(0) == Approx(1.1));
        
        // Test const access
        const Frame& const_frame = frame;
        const auto& const_ids = const_frame.operator()<int>("atoms", "id");
        REQUIRE(const_ids(0) == 1);
    }
    
    SECTION("Span access") {
        Block atoms;
        xt::xarray<int> ids = {1, 2, 3, 4, 5};
        xt::xarray<double> coords = {1.1, 2.2, 3.3, 4.4, 5.5};
        
        atoms.set("id", ids);
        atoms.set("coords", coords);
        frame.set_block("atoms", atoms);
        
        // Test get_variable_span
        auto id_span = frame.get_variable_span<int>("atoms", "id");
        auto coord_span = frame.get_variable_span<double>("atoms", "coords");
        
        REQUIRE(id_span.size() == 5);
        REQUIRE(coord_span.size() == 5);
        REQUIRE(id_span[0] == 1);
        REQUIRE(coord_span[0] == Approx(1.1));
        
        // Test const span access
        const auto& const_id_span = frame.get_variable_span<int>("atoms", "id");
        REQUIRE(const_id_span.size() == 5);
        REQUIRE(const_id_span[0] == 1);
    }
    
    SECTION("Validation") {
        // Valid frame
        Block atoms;
        xt::xarray<int> ids = {1, 2, 3};
        atoms.set("id", ids);
        frame.set_block("atoms", atoms);
        
        REQUIRE(frame.validate());
        REQUIRE_NOTHROW(frame.validate());
        
        // Invalid frame (empty block)
        Frame invalid_frame;
        REQUIRE(invalid_frame.validate());
        
        // Test to_string
        std::string frame_str = frame.to_string();
        REQUIRE(frame_str.find("Frame with 1 blocks") != std::string::npos);
        REQUIRE(frame_str.find("Block 'atoms': 1 variables") != std::string::npos);
    }
    
    SECTION("Error handling") {
        REQUIRE_THROWS_AS(frame.operator()<int>("nonexistent", "var"), std::out_of_range);
        REQUIRE_THROWS_AS(frame.operator()<int>("atoms", "nonexistent"), std::out_of_range);
        
        REQUIRE_THROWS_AS(frame.get_variable_span<int>("nonexistent", "var"), std::out_of_range);
        REQUIRE_THROWS_AS(frame.get_variable_span<int>("atoms", "nonexistent"), std::out_of_range);
        
        REQUIRE_THROWS_AS(frame.at("nonexistent"), std::out_of_range);
    }
    
    SECTION("Copy and move semantics") {
        Block atoms;
        xt::xarray<int> ids = {1, 2, 3};
        atoms.set("id", ids);
        frame.set_block("atoms", atoms);
        frame.set_metadata("title", "Test");
        
        // Copy constructor
        Frame frame_copy(frame);
        REQUIRE(frame_copy.get_block_count() == frame.get_block_count());
        REQUIRE(frame_copy.contains_block("atoms"));
        REQUIRE(frame_copy.has_metadata("title"));
        
        // Copy assignment
        Frame frame_copy2;
        frame_copy2 = frame;
        REQUIRE(frame_copy2.get_block_count() == frame.get_block_count());
        
        // Move constructor
        Frame frame_move(std::move(frame_copy));
        REQUIRE(frame_move.get_block_count() == 1);
        REQUIRE(frame_move.contains_block("atoms"));
        
        // Move assignment
        Frame frame_move2;
        frame_move2 = std::move(frame_move);
        REQUIRE(frame_move2.get_block_count() == 1);
    }
    
    SECTION("Factory functions") {
        Block atoms, bonds;
        xt::xarray<int> atom_ids = {1, 2, 3};
        xt::xarray<int> bond_ids = {1, 2};
        
        atoms.set("id", atom_ids);
        bonds.set("id", bond_ids);
        
        // Test make_frame with map
        std::map<std::string, Block> block_map;
        block_map["atoms"] = atoms;
        block_map["bonds"] = bonds;
        
        Frame frame_from_map = make_frame(block_map);
        REQUIRE(frame_from_map.get_block_count() == 2);
        REQUIRE(frame_from_map.contains_block("atoms"));
        REQUIRE(frame_from_map.contains_block("bonds"));
        
        // Test make_frame_from_blocks
        Frame frame_from_blocks = make_frame_from_blocks("atoms", atoms, bonds);
        REQUIRE(frame_from_blocks.get_block_count() == 2);
    }
    
    SECTION("Complex data structures") {
        // Create multiple blocks with different data types
        Block atoms, bonds, angles;
        
        xt::xarray<int> atom_ids = {1, 2, 3, 4, 5};
        xt::xarray<double> atom_coords = {1.1, 2.2, 3.3, 4.4, 5.5};
        xt::xarray<int> atom_types = {1, 2, 2, 2, 2}; // 1=C, 2=H
        
        xt::xarray<int> bond_ids = {1, 2, 3, 4};
        xt::xarray<int> bond_atoms = {1, 2, 1, 3, 1, 4, 1, 5};
        xt::xarray<double> bond_lengths = {1.09, 1.09, 1.09, 1.09};
        
        xt::xarray<int> angle_ids = {1, 2, 3, 4, 5, 6};
        xt::xarray<double> angle_values = {109.5, 109.5, 109.5, 109.5, 109.5, 109.5};
        
        atoms.set("id", atom_ids);
        atoms.set("coords", atom_coords);
        atoms.set("type", atom_types);
        
        bonds.set("id", bond_ids);
        bonds.set("atoms", bond_atoms);
        bonds.set("length", bond_lengths);
        
        angles.set("id", angle_ids);
        angles.set("value", angle_values);
        
        frame.set_block("atoms", atoms);
        frame.set_block("bonds", bonds);
        frame.set_block("angles", angles);
        
        REQUIRE(frame.get_block_count() == 3);
        REQUIRE(frame.n_variables() == 8); // 3 + 3 + 2
        
        // Test access to complex data
        auto& retrieved_atom_ids = frame.operator()<int>("atoms", "id");
        auto& retrieved_bond_lengths = frame.operator()<double>("bonds", "length");
        auto& retrieved_angle_values = frame.operator()<double>("angles", "value");
        
        REQUIRE(retrieved_atom_ids.size() == 5);
        REQUIRE(retrieved_bond_lengths.size() == 4);
        REQUIRE(retrieved_angle_values.size() == 6);
        
        REQUIRE(retrieved_atom_ids(0) == 1);
        REQUIRE(retrieved_bond_lengths(0) == Approx(1.09));
        REQUIRE(retrieved_angle_values(0) == Approx(109.5));
    }
    
    SECTION("Performance with large data") {
        const size_t n_atoms = 1000;
        const size_t n_bonds = n_atoms * 2;
        
        Block atoms, bonds;
        
        xt::xarray<int> atom_ids = xt::zeros<int>({n_atoms});
        xt::xarray<double> atom_coords = xt::zeros<double>({n_atoms});
        xt::xarray<int> bond_ids = xt::zeros<int>({n_bonds});
        xt::xarray<int> bond_atoms = xt::zeros<int>({n_bonds * 2});
        xt::xarray<double> bond_lengths = xt::zeros<double>({n_bonds});
        
        for (size_t i = 0; i < n_atoms; ++i) {
            atom_ids[i] = i;
            atom_coords[i] = i * 0.1;
        }
        
        for (size_t i = 0; i < n_bonds; ++i) {
            bond_ids[i] = i;
            bond_atoms[i * 2] = i;
            bond_atoms[i * 2 + 1] = (i + 1) % n_atoms;
            bond_lengths[i] = i * 0.01;
        }
        
        atoms.set("id", atom_ids);
        atoms.set("coords", atom_coords);
        
        bonds.set("id", bond_ids);
        bonds.set("atoms", bond_atoms);
        bonds.set("length", bond_lengths);
        
        frame.set_block("atoms", atoms);
        frame.set_block("bonds", bonds);
        
        REQUIRE(frame.get_block_count() == 2);
        REQUIRE(frame.n_variables() == 5);
        
        // Test random access performance
        for (size_t i = 0; i < n_atoms; i += 100) {
            REQUIRE(frame.operator()<int>("atoms", "id")(i) == static_cast<int>(i));
            REQUIRE(frame.operator()<double>("atoms", "coords")(i) == Approx(static_cast<double>(i) * 0.1));
        }
        
        for (size_t i = 0; i < n_bonds; i += 100) {
            REQUIRE(frame.operator()<int>("bonds", "id")(i) == static_cast<int>(i));
            REQUIRE(frame.operator()<double>("bonds", "length")(i) == Approx(static_cast<double>(i) * 0.01));
        }
    }
}
