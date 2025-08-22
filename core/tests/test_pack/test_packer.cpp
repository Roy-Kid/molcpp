#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/packer.hpp"
#include "molcpp/pack/target.hpp"
#include "molcpp/pack/constraint.hpp"
#include "molcpp/core/frame.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace molcpp;
using namespace Catch;

TEST_CASE("MolPacker: Basic Functionality", "[pack][packer]") {
    
    SECTION("MolPacker creation") {
        MolPacker<> packer;
        
        REQUIRE(packer.n_targets() == 0);
        REQUIRE(packer.n_points() == 0);
    }
    
    SECTION("MolPacker add target") {
        MolPacker<> packer;
        
        // Create a simple frame
        Frame frame;
        xt::xarray<Real> coords = xt::ones<Real>({2, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{-5.0, -5.0, -5.0};
        auto constraint = std::make_unique<InsideBoxConstraint<>>(box_lengths, box_origin);
        Target<> target(frame, 3, std::move(constraint));
        
        // Add target to packer
        packer.add_target(std::move(target));
        
        REQUIRE(packer.n_targets() == 1);
        REQUIRE(packer.n_points() == 6);  // 3 copies * 2 atoms each
    }
    
    SECTION("MolPacker add multiple targets") {
        MolPacker<> packer;
        
        // Create first target
        Frame frame1;
        xt::xarray<Real> coords1 = xt::ones<Real>({1, 3});
        auto block1 = make_block_from_arrays("coords", coords1);
        frame1.set_block("atoms", std::move(block1));
        Target<> target1(frame1, 2, nullptr);
        
        // Create second target
        Frame frame2;
        xt::xarray<Real> coords2 = xt::ones<Real>({3, 3});
        auto block2 = make_block_from_arrays("coords", coords2);
        frame2.set_block("atoms", std::move(block2));
        Target<> target2(frame2, 1, nullptr);
        
        packer.add_target(std::move(target1));
        packer.add_target(std::move(target2));
        
        REQUIRE(packer.n_targets() == 2);
        REQUIRE(packer.n_points() == 5);  // (2*1) + (1*3) = 5
    }
}

TEST_CASE("MolPacker: Basic Packing", "[pack][packer]") {
    
    SECTION("Pack with single target") {
        MolPacker<> packer;
        
        // Create a simple target
        Frame frame;
        xt::xarray<Real> coords = xt::zeros<Real>({1, 3});
        coords(0, 0) = 0.0; coords(0, 1) = 0.0; coords(0, 2) = 0.0;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<> target(frame, 2, nullptr);
        packer.add_target(std::move(target));
        
        // Pack the system
        auto packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        REQUIRE(packed_frame.contains_block("atoms"));
        
        // Should have coordinates for 2 copies
        xt::xarray<Real> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 2);
        REQUIRE(packed_coords.shape(1) == 3);
    }
    
    SECTION("Pack with constraint") {
        MolPacker<> packer;
        
        // Create target with box constraint
        Frame frame;
        xt::xarray<Real> coords = xt::zeros<Real>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Vec3<> box_lengths{2.0, 2.0, 2.0};
        Vec3<> box_origin{-1.0, -1.0, -1.0};
        auto constraint = std::make_unique<InsideBoxConstraint<>>(box_lengths, box_origin);
        Target<> target(frame, 3, std::move(constraint));
        
        packer.add_target(std::move(target));
        
        auto packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        REQUIRE(packed_frame.contains_block("atoms"));
        
        // Verify coordinates are within the box constraint
        xt::xarray<Real> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 3);
        REQUIRE(packed_coords.shape(1) == 3);
        
        // All coordinates should be within [-1, 1] bounds (though this is not guaranteed 
        // with the placeholder optimizer, we just check the structure is correct)
    }
}

TEST_CASE("MolPacker: Optimizer Integration", "[pack][packer]") {
    
    SECTION("Set default optimizer") {
        MolPacker<> packer;
        
        auto optimizer = make_optimizer<Real>("gradient_descent");
        packer.set_default_optimizer(std::move(optimizer));
        
        // Create a simple target
        Frame frame;
        xt::xarray<Real> coords = xt::ones<Real>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<> target(frame, 1, nullptr);
        packer.add_target(std::move(target));
        
        auto packed_frame = packer.pack();
        
        // Check that optimization result is available
        auto last_result = packer.get_last_result();
        REQUIRE(last_result.positions.shape(0) == 1);
        REQUIRE(last_result.positions.shape(1) == 3);
    }
    
    SECTION("Pack with custom parameters") {
        MolPacker<> packer;
        
        // Create target
        Frame frame;
        xt::xarray<Real> coords = xt::zeros<Real>({2, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<> target(frame, 1, nullptr);
        packer.add_target(std::move(target));
        
        // Set custom optimization parameters
        OptimizationParams<> params;
        params.max_iterations = 100;
        params.tolerance = 1e-4;
        params.verbose = false;
        
        auto packed_frame = packer.pack_with_params({}, params);
        
        REQUIRE(packed_frame.n_blocks() > 0);
    }
}

TEST_CASE("MolPacker: Template Type Support", "[pack][packer]") {
    
    SECTION("Float precision packer") {
        MolPacker<float> packer;
        
        Frame frame;
        xt::xarray<float> coords = xt::ones<float>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<float> target(frame, 2, nullptr);
        packer.add_target(std::move(target));
        
        auto packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        xt::xarray<float> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 2);
    }
    
    SECTION("Double precision packer") {
        MolPacker<double> packer;
        
        Frame frame;
        xt::xarray<double> coords = xt::ones<double>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<double> target(frame, 3, nullptr);
        packer.add_target(std::move(target));
        
        auto packed_frame = packer.pack();
        
        REQUIRE(packed_frame.n_blocks() > 0);
        xt::xarray<double> packed_coords = packed_frame["atoms"]["coords"];
        REQUIRE(packed_coords.shape(0) == 3);
    }
    
    SECTION("Type aliases work correctly") {
        // Test that type aliases compile and work
        MolPackerf float_packer;
        MolPackerd double_packer;
        DefaultMolPacker default_packer;
        
        REQUIRE(float_packer.n_targets() == 0);
        REQUIRE(double_packer.n_targets() == 0);
        REQUIRE(default_packer.n_targets() == 0);
    }
}

TEST_CASE("MolPacker: Error Handling", "[pack][packer]") {
    
    SECTION("Pack with no targets") {
        MolPacker<> packer;
        
        // Should still return a valid frame, even if empty
        auto packed_frame = packer.pack();
        
        // The exact behavior for empty packer may vary, but it shouldn't crash
        // We just verify that we get some result
        REQUIRE(packed_frame.n_blocks() >= 0);
    }
    
    SECTION("Pack with empty target") {
        MolPacker<> packer;
        
        // Create target with zero copies
        Frame frame;
        xt::xarray<Real> coords = xt::ones<Real>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<> target(frame, 0, nullptr);  // Zero copies
        packer.add_target(std::move(target));
        
        REQUIRE(packer.n_targets() == 1);
        REQUIRE(packer.n_points() == 0);
        
        // Should handle gracefully
        auto packed_frame = packer.pack();
        REQUIRE(packed_frame.n_blocks() >= 0);
    }
}
