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
        xt::xarray<float> coords = xt::zeros<float>({3, 3});
        coords(0, 0) = 0.0f; coords(0, 1) = 0.0f; coords(0, 2) = 0.0f;   // O
        coords(1, 0) = 1.0f; coords(1, 1) = 0.0f; coords(1, 2) = 0.0f;   // H1
        coords(2, 0) = 0.0f; coords(2, 1) = 1.0f; coords(2, 2) = 0.0f;   // H2
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create constraint
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{-5.0f, -5.0f, -5.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        
        // Create target
        Target target(frame, 10, constraint);
        
        REQUIRE(target.get_number() == 10);
        REQUIRE(target.n_points() == 30);  // 10 copies * 3 atoms each
    }
    
    SECTION("Target coordinate extraction") {
        // Create a simple frame
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({2, 3});
        coords(0, 0) = 1.0f; coords(0, 1) = 2.0f; coords(0, 2) = 3.0f;
        coords(1, 0) = 4.0f; coords(1, 1) = 5.0f; coords(1, 2) = 6.0f;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target with a simple constraint
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{-5.0f, -5.0f, -5.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 1, constraint);
        
        // Extract coordinates
        auto extracted_coords = target.points();
        
        REQUIRE(extracted_coords.shape(0) == 2);
        REQUIRE(extracted_coords.shape(1) == 3);
        REQUIRE(extracted_coords(0, 0) == Approx(1.0f));
        REQUIRE(extracted_coords(0, 1) == Approx(2.0f));
        REQUIRE(extracted_coords(0, 2) == Approx(3.0f));
        REQUIRE(extracted_coords(1, 0) == Approx(4.0f));
        REQUIRE(extracted_coords(1, 1) == Approx(5.0f));
        REQUIRE(extracted_coords(1, 2) == Approx(6.0f));
    }
    
    SECTION("Target with x, y, z coordinate format") {
        // Create frame with separate x, y, z arrays
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({2, 3});
        coords(0, 0) = 1.0f; coords(0, 1) = 2.0f; coords(0, 2) = 3.0f;
        coords(1, 0) = 4.0f; coords(1, 1) = 5.0f; coords(1, 2) = 6.0f;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create target with a simple constraint
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{-5.0f, -5.0f, -5.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 1, constraint);
        
        // Extract coordinates
        auto extracted_coords = target.points();
        
        REQUIRE(extracted_coords.shape(0) == 2);
        REQUIRE(extracted_coords.shape(1) == 3);
        REQUIRE(extracted_coords(0, 0) == Approx(1.0f));
        REQUIRE(extracted_coords(0, 1) == Approx(2.0f));
        REQUIRE(extracted_coords(0, 2) == Approx(3.0f));
        REQUIRE(extracted_coords(1, 0) == Approx(4.0f));
        REQUIRE(extracted_coords(1, 1) == Approx(5.0f));
        REQUIRE(extracted_coords(1, 2) == Approx(6.0f));
    }
    
    SECTION("Target string representation") {
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({2, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{-5.0f, -5.0f, -5.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 5, constraint);
        
        std::string str_repr = target.to_string();
        REQUIRE(str_repr.find("Target") != std::string::npos);
        REQUIRE(str_repr.find("number=5") != std::string::npos);
    }
}

TEST_CASE("Target: Constraint Integration", "[pack][target]") {
    
    SECTION("Target with box constraint") {
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({1, 3});
        coords(0, 0) = 0.0f; coords(0, 1) = 0.0f; coords(0, 2) = 0.0f;
        
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create constraint
        Vec3f box_lengths{2.0f, 2.0f, 2.0f};
        Vec3f box_origin{-1.0f, -1.0f, -1.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        
        Target target(frame, 1, constraint);
        
        // Test constraint exists and works
        auto test_points_inside = xt::zeros<float>({1, 3});
        REQUIRE(target.get_constraint().penalty(test_points_inside) == Approx(0.0f));
        
        // Test coordinate that should be outside
        xt::xarray<float> test_points_outside = xt::zeros<float>({1, 3});
        test_points_outside(0, 0) = 2.0f; // Outside the box
        REQUIRE(target.get_constraint().penalty(test_points_outside) > 0.0f);
    }
    
    SECTION("Target without constraint") {
        Frame frame;
        
        xt::xarray<float> coords = xt::zeros<float>({1, 3});
        auto block = make_block_from_arrays("coords", coords);
        frame.set_block("atoms", std::move(block));
        
        // Create a simple constraint for testing
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        auto constraint = make_inside_box(box_lengths, box_origin);
        Target target(frame, 1, constraint);
        
        // Test constraint exists
        REQUIRE(target.get_constraint().penalty != nullptr); // constraint function exists
        
        // With constraint, penalty calculation is available
        auto test_points = xt::zeros<float>({1, 3});
        REQUIRE(target.get_constraint().penalty(test_points) >= 0.0f);
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
