#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/constraint.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace Catch;

TEST_CASE("Constraint: InsideBoxConstraint", "[pack][constraint]") {
    
    SECTION("Creation and basic properties") {
        Vec3<Real> box_lengths{10.0, 10.0, 10.0};
        Vec3<Real> box_origin{0.0, 0.0, 0.0};
        
        InsideBoxConstraint<Real> constraint(box_lengths, box_origin);
        REQUIRE(constraint.penalty(xt::zeros<Real>({1, 3})) == Approx(0.0));
    }
    
    SECTION("Penalty calculation") {
        Vec3<Real> box_lengths{10.0, 10.0, 10.0};
        Vec3<Real> box_origin{0.0, 0.0, 0.0};
        
        InsideBoxConstraint<Real> constraint(box_lengths, box_origin);
        
        // Point inside the box should have zero penalty
        xt::xarray<Real> inside_points = xt::zeros<Real>({1, 3});
        inside_points(0, 0) = 5.0; inside_points(0, 1) = 5.0; inside_points(0, 2) = 5.0;
        REQUIRE(constraint.penalty(inside_points) == Approx(0.0));
        
        // Point outside the box should have non-zero penalty  
        xt::xarray<Real> outside_points = xt::zeros<Real>({1, 3});
        outside_points(0, 0) = 15.0; outside_points(0, 1) = 5.0; outside_points(0, 2) = 5.0;
        REQUIRE(constraint.penalty(outside_points) > 0.0);
    }
    
    SECTION("Gradient calculation") {
        Vec3<Real> box_lengths{10.0, 10.0, 10.0};
        Vec3<Real> box_origin{0.0, 0.0, 0.0};
        
        InsideBoxConstraint<Real> constraint(box_lengths, box_origin);
        
        // Point inside the box should have zero gradient
        xt::xarray<Real> inside_points = xt::zeros<Real>({1, 3});
        inside_points(0, 0) = 5.0; inside_points(0, 1) = 5.0; inside_points(0, 2) = 5.0;
        auto grad_inside = constraint.dpenalty(inside_points);
        REQUIRE(grad_inside(0, 0) == Approx(0.0));
        REQUIRE(grad_inside(0, 1) == Approx(0.0));
        REQUIRE(grad_inside(0, 2) == Approx(0.0));
        
        // Point outside the box should have non-zero gradient pointing inward
        xt::xarray<Real> outside_points = xt::zeros<Real>({1, 3});
        outside_points(0, 0) = 15.0; outside_points(0, 1) = 5.0; outside_points(0, 2) = 5.0;
        auto grad_outside = constraint.dpenalty(outside_points);
        REQUIRE(grad_outside(0, 0) < 0.0);  // Should point toward the box (negative x direction)
        REQUIRE(grad_outside(0, 1) == Approx(0.0));  // y is within bounds
        REQUIRE(grad_outside(0, 2) == Approx(0.0));  // z is within bounds
    }
}

TEST_CASE("Constraint: Template Type Support", "[pack][constraint]") {
    
    SECTION("Float precision constraints") {
        Vec3<float> box_lengths{10.0f, 10.0f, 10.0f};
        Vec3<float> box_origin{0.0f, 0.0f, 0.0f};
        
        InsideBoxConstraint<float> constraint(box_lengths, box_origin);
        
        xt::xarray<float> test_points = xt::zeros<float>({1, 3});
        test_points(0, 0) = 5.0f; test_points(0, 1) = 5.0f; test_points(0, 2) = 5.0f;
        REQUIRE(constraint.penalty(test_points) == Approx(0.0f));
    }
    
    SECTION("Double precision constraints") {
        Vec3<double> box_lengths{10.0, 10.0, 10.0};
        Vec3<double> box_origin{0.0, 0.0, 0.0};
        
        InsideBoxConstraint<double> constraint(box_lengths, box_origin);
        
        xt::xarray<double> test_points = xt::zeros<double>({1, 3});
        test_points(0, 0) = 5.0; test_points(0, 1) = 5.0; test_points(0, 2) = 5.0;
        REQUIRE(constraint.penalty(test_points) == Approx(0.0));
    }
    
    SECTION("Default precision (Real) constraints") {
        Vec3<> box_lengths{10.0, 10.0, 10.0};
        Vec3<> box_origin{0.0, 0.0, 0.0};
        
        InsideBoxConstraint<Real> constraint(box_lengths, box_origin);
        
        xt::xarray<Real> test_points = xt::zeros<Real>({1, 3});
        test_points(0, 0) = 5.0; test_points(0, 1) = 5.0; test_points(0, 2) = 5.0;
        REQUIRE(constraint.penalty(test_points) == Approx(0.0));
    }
}
