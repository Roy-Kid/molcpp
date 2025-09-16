#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/constraint.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace Catch;

TEST_CASE("Constraint: InsideBoxConstraint", "[pack][constraint]") {
    
    SECTION("Creation and basic properties") {
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_inside_box(box_lengths, box_origin);
        REQUIRE(constraint.penalty(xt::zeros<float>({1, 3})) == Approx(0.0f));
    }
    
    SECTION("Penalty calculation") {
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_inside_box(box_lengths, box_origin);
        
        // Point inside the box should have zero penalty
        xt::xarray<float> inside_points = xt::zeros<float>({1, 3});
        inside_points(0, 0) = 5.0f; inside_points(0, 1) = 5.0f; inside_points(0, 2) = 5.0f;
        REQUIRE(constraint.penalty(inside_points) == Approx(0.0f));
        
        // Point outside the box should have non-zero penalty  
        xt::xarray<float> outside_points = xt::zeros<float>({1, 3});
        outside_points(0, 0) = 15.0f; outside_points(0, 1) = 5.0f; outside_points(0, 2) = 5.0f;
        REQUIRE(constraint.penalty(outside_points) > 0.0f);
    }
    
    SECTION("Gradient calculation") {
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_inside_box(box_lengths, box_origin);
        
        // Point inside the box should have zero gradient
        xt::xarray<float> inside_points = xt::zeros<float>({1, 3});
        inside_points(0, 0) = 5.0f; inside_points(0, 1) = 5.0f; inside_points(0, 2) = 5.0f;
        auto grad_inside = constraint.dpenalty(inside_points);
        REQUIRE(grad_inside(0, 0) == Approx(0.0f));
        REQUIRE(grad_inside(0, 1) == Approx(0.0f));
        REQUIRE(grad_inside(0, 2) == Approx(0.0f));
        
        // Point outside the box should have non-zero gradient pointing inward
        xt::xarray<float> outside_points = xt::zeros<float>({1, 3});
        outside_points(0, 0) = 15.0f; outside_points(0, 1) = 5.0f; outside_points(0, 2) = 5.0f;
        auto grad_outside = constraint.dpenalty(outside_points);
        REQUIRE(grad_outside(0, 0) < 0.0f);  // Should point toward the box (negative x direction)
        REQUIRE(grad_outside(0, 1) == Approx(0.0f));  // y is within bounds
        REQUIRE(grad_outside(0, 2) == Approx(0.0f));  // z is within bounds
    }
}

TEST_CASE("Constraint: Factory Function Support", "[pack][constraint]") {
    
    SECTION("Float precision constraints") {
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_inside_box(box_lengths, box_origin);
        
        xt::xarray<float> test_points = xt::zeros<float>({1, 3});
        test_points(0, 0) = 5.0f; test_points(0, 1) = 5.0f; test_points(0, 2) = 5.0f;
        REQUIRE(constraint.penalty(test_points) == Approx(0.0f));
    }
    
    SECTION("Outside box constraints") {
        Vec3f box_lengths{10.0f, 10.0f, 10.0f};
        Vec3f box_origin{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_outside_box(box_origin, box_lengths);
        
        // Point inside the box should have positive penalty
        xt::xarray<float> inside_points = xt::zeros<float>({1, 3});
        inside_points(0, 0) = 5.0f; inside_points(0, 1) = 5.0f; inside_points(0, 2) = 5.0f;
        REQUIRE(constraint.penalty(inside_points) > 0.0f);
    }
    
    SECTION("Sphere constraints") {
        float radius = 5.0f;
        Vec3f center{0.0f, 0.0f, 0.0f};
        
        auto constraint = make_inside_sphere(radius, center);
        
        // Point at origin should have zero penalty
        xt::xarray<float> origin_points = xt::zeros<float>({1, 3});
        REQUIRE(constraint.penalty(origin_points) == Approx(0.0f));
    }
}
