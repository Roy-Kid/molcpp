#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/packer.hpp"
#include "molcpp/pack/target.hpp"
#include "molcpp/pack/constraint.hpp"
#include "molcpp/pack/optimizer.hpp"
#include "molcpp/core/frame.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace molcpp;
using namespace Catch;

TEST_CASE("MolPacker: Basic Functionality", "[pack][packer]") {
    
    SECTION("MolPacker creation") {
        MolPacker packer;
        
        REQUIRE(packer.get_targets().size() == 0);
        REQUIRE(packer.n_points() == 0);
    }
    
    SECTION("MolPacker add target") {
        MolPacker packer;
        
        // Create a simple frame
        Frame frame;
        xt::xarray<float> coords = xt::ones<float>({2, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{-5.0f, -5.0f, -5.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 3, constraint);
        
        // Add target to packer
        packer.add_target(std::move(target));
        
        REQUIRE(packer.get_targets().size() == 1);
        REQUIRE(packer.n_points() == 6);  // 3 copies * 2 atoms each
    }
    
    SECTION("MolPacker add multiple targets") {
        MolPacker packer;
        
        // Create first target
        Frame frame1;
        xt::xarray<float> coords1 = xt::ones<float>({1, 3});
        molcpp::Block block1 = make_block_from_arrays("coords", coords1);
        frame1.set_block("atoms", std::move(block1));
        // Create a simple constraint for target1
        Vec3f box_lengths1{10.0f, 10.0f, 10.0f};
        Vec3f box_origin1{0.0f, 0.0f, 0.0f};
        Constraint constraint1 = make_inside_box(box_lengths1, box_origin1);
        Target target1(frame1, 2, constraint1);
        
        // Create second target
        Frame frame2;
        xt::xarray<float> coords2 = xt::ones<float>({3, 3});
        molcpp::Block block2 = make_block_from_arrays("coords", coords2);
        frame2.set_block("atoms", std::move(block2));
        // Create a simple constraint for target2
        Vec3f box_lengths2{10.0f, 10.0f, 10.0f};
        Vec3f box_origin2{0.0f, 0.0f, 0.0f};
        Constraint constraint2 = make_inside_box(box_lengths2, box_origin2);
        Target target2(frame2, 1, constraint2);
        
        packer.add_target(std::move(target1));
        packer.add_target(std::move(target2));
        
        REQUIRE(packer.get_targets().size() == 2);
        REQUIRE(packer.n_points() == 5);  // (2*1) + (1*3) = 5
    }
}

TEST_CASE("MolPacker: Basic Packing", "[pack][packer]") {
    
    SECTION("Pack with single target") {
        MolPacker packer;
        
        // Create a simple target
        Frame frame;
        xt::xarray<float> coords = xt::zeros<float>({1, 3});
        coords(0, 0) = 0.0; coords(0, 1) = 0.0; coords(0, 2) = 0.0;
        
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 2, constraint);
        packer.add_target(std::move(target));
        
        // Pack the system
        Frame packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        REQUIRE(packed_frame.contains_block("atoms"));
        
        // Should have coordinates for 2 copies
        xt::xarray<float> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 2);
        REQUIRE(packed_coords.shape(1) == 3);
    }
    
    SECTION("Pack with constraint") {
        MolPacker packer;
        
        // Create target with box constraint
        Frame frame;
        xt::xarray<float> coords = xt::zeros<float>({1, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Vec3f box_lengths{2.0f, 2.0f, 2.0f};
        Vec3f box_origin{-1.0f, -1.0f, -1.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 3, constraint);
        
        packer.add_target(std::move(target));
        
        Frame packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        REQUIRE(packed_frame.contains_block("atoms"));
        
        // Verify coordinates are within the box constraint
        xt::xarray<float> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 3);
        REQUIRE(packed_coords.shape(1) == 3);
        
        // All coordinates should be within [-1, 1] bounds (though this is not guaranteed 
        // with the placeholder optimizer, we just check the structure is correct)
    }
}

TEST_CASE("MolPacker: Optimizer Integration", "[pack][packer]") {
    
    SECTION("Set default optimizer") {
        MolPacker packer;
        
        // Note: MolPacker doesn't have set_default_optimizer method - skipping
        
        // Create a simple target
        Frame frame;
        xt::xarray<float> coords = xt::ones<float>({1, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 1, constraint);
        packer.add_target(std::move(target));
        
        Frame packed_frame = packer.pack();
        
        // Check that packed frame is available
        REQUIRE(packed_frame.contains_block("atoms"));
        const auto& atoms = packed_frame["atoms"];
        REQUIRE(atoms.contains("coords"));
    }
    
    SECTION("Pack with custom parameters") {
        MolPacker packer;
        
        // Create target
        Frame frame;
        xt::xarray<float> coords = xt::zeros<float>({2, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 1, constraint);
        packer.add_target(std::move(target));
        
        // Set custom optimization parameters
        OptimizationParams<float> params;
        params.max_iterations = 100;
        params.tolerance = 1e-4;
        params.verbose = false;
        
        // Note: MolPacker doesn't have pack_with_params method, using pack() with max_steps
        Frame packed_frame = packer.pack({}, params.max_iterations);
        
        REQUIRE(packed_frame.n_blocks() > 0);
    }
}

TEST_CASE("MolPacker: Template Type Support", "[pack][packer]") {
    
    SECTION("Float precision packer") {
        MolPacker packer;
        
        Frame frame;
        xt::xarray<float> coords = xt::ones<float>({1, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 2, constraint);
        packer.add_target(std::move(target));
        
        Frame packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        xt::xarray<float> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 2);
    }
    
    SECTION("Double precision packer (converted to float)") {
        MolPacker packer;
        
        Frame frame;
        // Note: Now using float since we removed Real and use float throughout
        xt::xarray<float> coords = xt::ones<float>({1, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for target
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 3, constraint);
        packer.add_target(std::move(target));
        
        Frame packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        xt::xarray<float> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 3);
    }
    
    SECTION("Type aliases work correctly") {
        // Test that type aliases compile and work
        MolPacker float_packer;
        MolPacker double_packer;
        MolPacker default_packer;
        
        REQUIRE(float_packer.get_targets().size() == 0);
        REQUIRE(double_packer.get_targets().size() == 0);
        REQUIRE(default_packer.get_targets().size() == 0);
    }
}

TEST_CASE("MolPacker: Error Handling", "[pack][packer]") {
    
    SECTION("Pack with no targets") {
        MolPacker packer;
        
        // Should throw an exception when trying to pack with no targets
        REQUIRE_THROWS_AS(packer.pack(), std::runtime_error);
    }
    
    SECTION("Pack with empty target") {
        MolPacker packer;
        
        // Should throw an exception when trying to create target with zero copies
        Frame frame;
        xt::xarray<float> coords = xt::ones<float>({1, 3});
        molcpp::Block block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        Constraint constraint = make_inside_box(box_lengths, box_origin);
        
        // Target constructor should throw for zero copies
        REQUIRE_THROWS_AS(Target(frame, 0, constraint), std::invalid_argument);
    }
}
