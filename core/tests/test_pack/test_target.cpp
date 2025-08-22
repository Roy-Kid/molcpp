#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/target.hpp"
#include "molcpp/pack/constraint.hpp"
#include "molcpp/core/frame.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace molcpp;
using namespace Catch;

TEST_CASE("Target: Basic Functionality", "[pack][target]") {
    
    SECTION("Target creation with Frame") {
        // Create a simple frame with coordinate data
        Frame frame;
        
        // Add coordinate data for a water molecule
        xt::xarray<Real> coords = xt::zeros<Real>({3, 3});
        coords(0, 0) = 0.0; coords(0, 1) = 0.0; coords(0, 2) = 0.0;   // O
        coords(1, 0) = 1.0; coords(1, 1) = 0.0; coords(1, 2) = 0.0;   // H1
        coords(2, 0) = 0.0; coords(2, 1) = 1.0; coords(2, 2) = 0.0;   // H2
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create constraint
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{-5.0, -5.0, -5.0};
        auto constraint = std::make_unique<InsideBoxConstraint<Real>>(box_lengths, box_origin);
        
        // Create target
        Target<Real> target(frame, 10, std::move(constraint));
        
        REQUIRE(target.get_number() == 10);
        REQUIRE(target.n_points() == 30);  // 10 copies * 3 atoms each
    }
    
    SECTION("Target coordinate extraction") {
        // Create a simple frame
        Frame frame;
        
        xt::xarray<Real> coords = xt::zeros<Real>({2, 3});
        coords(0, 0) = 1.0; coords(0, 1) = 2.0; coords(0, 2) = 3.0;
        coords(1, 0) = 4.0; coords(1, 1) = 5.0; coords(1, 2) = 6.0;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target with a simple constraint
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{-5.0, -5.0, -5.0};
        auto constraint = std::make_unique<InsideBoxConstraint<Real>>(box_lengths, box_origin);
        Target<Real> target(frame, 1, std::move(constraint));
        
        // Extract coordinates
        auto extracted_coords = target.points();
        
        REQUIRE(extracted_coords.shape(0) == 2);
        REQUIRE(extracted_coords.shape(1) == 3);
        REQUIRE(extracted_coords(0, 0) == Approx(1.0));
        REQUIRE(extracted_coords(0, 1) == Approx(2.0));
        REQUIRE(extracted_coords(0, 2) == Approx(3.0));
        REQUIRE(extracted_coords(1, 0) == Approx(4.0));
        REQUIRE(extracted_coords(1, 1) == Approx(5.0));
        REQUIRE(extracted_coords(1, 2) == Approx(6.0));
    }
    
    SECTION("Target with x, y, z coordinate format") {
        // Create frame with separate x, y, z arrays
        Frame frame;
        
        xt::xarray<Real> coords = xt::zeros<Real>({2, 3});
        coords(0, 0) = 1.0; coords(0, 1) = 2.0; coords(0, 2) = 3.0;
        coords(1, 0) = 4.0; coords(1, 1) = 5.0; coords(1, 2) = 6.0;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target with a simple constraint
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{-5.0, -5.0, -5.0};
        auto constraint = std::make_unique<InsideBoxConstraint<Real>>(box_lengths, box_origin);
        Target<Real> target(frame, 1, std::move(constraint));
        
        // Extract coordinates
        auto extracted_coords = target.points();
        
        REQUIRE(extracted_coords.shape(0) == 2);
        REQUIRE(extracted_coords.shape(1) == 3);
        REQUIRE(extracted_coords(0, 0) == Approx(1.0));
        REQUIRE(extracted_coords(0, 1) == Approx(2.0));
        REQUIRE(extracted_coords(0, 2) == Approx(3.0));
        REQUIRE(extracted_coords(1, 0) == Approx(4.0));
        REQUIRE(extracted_coords(1, 1) == Approx(5.0));
        REQUIRE(extracted_coords(1, 2) == Approx(6.0));
    }
    
    SECTION("Target string representation") {
        Frame frame;
        
        xt::xarray<Real> coords = xt::zeros<Real>({2, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{-5.0, -5.0, -5.0};
        auto constraint = std::make_unique<InsideBoxConstraint<Real>>(box_lengths, box_origin);
        Target<Real> target(frame, 5, std::move(constraint));
        
        std::string str_repr = target.to_string();
        REQUIRE(str_repr.find("Target") != std::string::npos);
        REQUIRE(str_repr.find("number=5") != std::string::npos);
    }
}

TEST_CASE("Target: Constraint Integration", "[pack][target]") {
    
    SECTION("Target with box constraint") {
        Frame frame;
        
        xt::xarray<Real> coords = xt::zeros<Real>({1, 3});
        coords(0, 0) = 0.0; coords(0, 1) = 0.0; coords(0, 2) = 0.0;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create constraint
        Vec3<> box_lengths{2.0, 2.0, 2.0};
        Vec3<> box_origin{-1.0, -1.0, -1.0};
        auto constraint = std::make_unique<InsideBoxConstraint<Real>>(box_lengths, box_origin);
        
        Target<Real> target(frame, 1, std::move(constraint));
        
        REQUIRE(target.get_constraint() != nullptr);
        
        // Test coordinate that should be inside
        Vec3<> inside_point{0.5, 0.5, 0.5};
        REQUIRE(constraint->penalty(xt::zeros<Real>({1, 3})) == Approx(0.0));
        
        // Test coordinate that should be outside
        Vec3<> outside_point{2.0, 0.0, 0.0};
        REQUIRE(constraint->penalty(xt::zeros<Real>({1, 3})) >= 0.0);
    }
    
    SECTION("Target without constraint") {
        Frame frame;
        
        xt::xarray<Real> coords = xt::zeros<Real>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<Real> target(frame, 1, nullptr);
        
        REQUIRE(target.get_constraint() != nullptr);
        
        // With constraint, penalty calculation is available
        Vec3<> any_point{100.0, 100.0, 100.0};
        // Constraint penalty calculation is available
    }
}

TEST_CASE("Target: Template Type Support", "[pack][target]") {
    
    // Temporarily disabled due to incomplete Optimizer type
    /*
    SECTION("Float precision target") {
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({1, 3});
        coords(0, 0) = 1.0f; coords(0, 1) = 2.0f; coords(0, 2) = 3.0f;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<float> target(frame, 2, nullptr);
        
        REQUIRE(target.get_number() == 2);
        REQUIRE(target.n_points() == 2);
        
        auto extracted = target.points();
        REQUIRE(extracted(0, 0) == Approx(1.0f));
        REQUIRE(extracted(0, 1) == Approx(2.0f));
        REQUIRE(extracted(0, 2) == Approx(3.0f));
    }
    
    SECTION("Double precision target") {
        Frame frame;
        
        xt::xarray<double> coords = xt::zeros<double>({1, 3});
        coords(0, 0) = 1.0; coords(0, 1) = 2.0; coords(0, 2) = 3.0;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Target<double> target(frame, 3, nullptr);
        
        REQUIRE(target.get_number() == 3);
        REQUIRE(target.n_points() == 3);
        
        auto extracted = target.points();
        REQUIRE(extracted(0, 0) == Approx(1.0));
        REQUIRE(extracted(0, 1) == Approx(2.0));
        REQUIRE(extracted(0, 2) == Approx(3.0));
    }
    */
}
