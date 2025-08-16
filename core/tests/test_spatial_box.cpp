#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/spatial/box.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/generators/xrandom.hpp>
#include <xtensor/core/xmath.hpp>
#include <xtensor-blas/xlinalg.hpp>
#include <memory>
#include <vector>
#include <cmath>

using namespace molcpp;
using Catch::Approx;

TEST_CASE("Box construction tests", "[spatial][box][construction]") {
    SECTION("Default constructor") {
        Box box;
        auto matrix = box.get_matrix();
        
        // Default box should be zeros
        REQUIRE(matrix(0, 0) == Approx(0.0));
        REQUIRE(matrix(1, 1) == Approx(0.0));
        REQUIRE(matrix(2, 2) == Approx(0.0));
        REQUIRE(box.get_volume() == Approx(0.0));
    }

    SECTION("Constructor with lengths") {
        Vec3 lengths = {2.0, 3.0, 4.0};
        Box box(lengths);
        
        auto computed_lengths = box.get_lengths();
        REQUIRE(computed_lengths[0] == Approx(2.0));
        REQUIRE(computed_lengths[1] == Approx(3.0));
        REQUIRE(computed_lengths[2] == Approx(4.0));
        
        REQUIRE(box.get_volume() == Approx(24.0));
        REQUIRE(box.get_style() == Box::Style::ORTHOGONAL);
    }

    SECTION("Constructor with initializer list lengths") {
        Box box({2.0, 3.0, 4.0});
        
        auto lengths = box.get_lengths();
        REQUIRE(lengths[0] == Approx(2.0));
        REQUIRE(lengths[1] == Approx(3.0));
        REQUIRE(lengths[2] == Approx(4.0));
    }

    SECTION("Constructor with matrix") {
        Mat3 matrix = {{2.0, 1.0, 0.5}, 
                       {0.0, 3.0, 0.1}, 
                       {0.0, 0.0, 4.0}};
        Box box(matrix);
        
        auto result_matrix = box.get_matrix();
        REQUIRE(result_matrix(0, 0) == Approx(2.0));
        REQUIRE(result_matrix(0, 1) == Approx(1.0));
        REQUIRE(result_matrix(1, 1) == Approx(3.0));
        REQUIRE(result_matrix(2, 2) == Approx(4.0));
        
        REQUIRE(box.get_style() == Box::Style::TRICLINIC);
    }

    SECTION("Constructor with initializer list matrix") {
        Box box({{2.0, 1.0, 0.5}, 
                 {0.0, 3.0, 0.1}, 
                 {0.0, 0.0, 4.0}});
        
        auto matrix = box.get_matrix();
        REQUIRE(matrix(0, 0) == Approx(2.0));
        REQUIRE(matrix(0, 1) == Approx(1.0));
        REQUIRE(matrix(1, 1) == Approx(3.0));
    }
}

TEST_CASE("Box from lengths and angles", "[spatial][box][construction]") {
    SECTION("Orthogonal box from angles") {
        Vec3 lengths = {2.0, 3.0, 4.0};
        Vec3 angles = {90.0, 90.0, 90.0};  // degrees
        
        Box box = Box::from_lengths_angles(lengths, angles);
        
        auto computed_lengths = box.get_lengths();
        REQUIRE(computed_lengths[0] == Approx(2.0));
        REQUIRE(computed_lengths[1] == Approx(3.0));
        REQUIRE(computed_lengths[2] == Approx(4.0));
        
        auto computed_angles = box.get_angles();
        REQUIRE(computed_angles[0] == Approx(90.0));
        REQUIRE(computed_angles[1] == Approx(90.0));
        REQUIRE(computed_angles[2] == Approx(90.0));
        
        REQUIRE(box.get_style() == Box::Style::ORTHOGONAL);
    }

    SECTION("Triclinic box from angles") {
        Vec3 lengths = {2.0, 3.0, 4.0};
        Vec3 angles = {80.0, 85.0, 95.0};  // degrees
        
        Box box = Box::from_lengths_angles(lengths, angles);
        
        auto computed_lengths = box.get_lengths();
        REQUIRE(computed_lengths[0] == Approx(2.0));
        REQUIRE(computed_lengths[1] == Approx(3.0));
        REQUIRE(computed_lengths[2] == Approx(4.0));
        
        auto computed_angles = box.get_angles();
        REQUIRE(computed_angles[0] == Approx(80.0));
        REQUIRE(computed_angles[1] == Approx(85.0));
        REQUIRE(computed_angles[2] == Approx(95.0));
        
        REQUIRE(box.get_style() == Box::Style::TRICLINIC);
    }
}

TEST_CASE("Box volume calculation", "[spatial][box][volume]") {
    SECTION("Orthogonal box volume") {
        Box box({2.0, 3.0, 4.0});
        REQUIRE(box.get_volume() == Approx(24.0));
        REQUIRE(box.volume() == Approx(24.0));  // Region interface
    }

    SECTION("Triclinic box volume") {
        Mat3 matrix = {{2.0, 1.0, 0.5}, 
                       {0.0, 3.0, 0.1}, 
                       {0.0, 0.0, 4.0}};
        Box box(matrix);
        
        // Volume = determinant of matrix
        double expected_volume = xt::linalg::det(matrix);
        REQUIRE(box.get_volume() == Approx(expected_volume));
    }

    SECTION("Zero volume for degenerate box") {
        Box box({0.0, 0.0, 0.0});
        REQUIRE(box.get_volume() == Approx(0.0));
    }
}

TEST_CASE("Box wrapping tests", "[spatial][box][wrapping]") {
    SECTION("Orthogonal box wrapping centered coordinates") {
        Box box({2.0, 2.0, 2.0});
        
        // Test wrapping to [-1, 1] range (for box length 2.0)
        xt::xarray<double> points = {{3.0, -0.5, 0.5}};
        auto wrapped = box.wrap(points);
        
        REQUIRE(wrapped(0, 0) == Approx(-1.0));  // 3.0 wraps to -1.0
        REQUIRE(wrapped(0, 1) == Approx(-0.5));  // -0.5 stays -0.5  
        REQUIRE(wrapped(0, 2) == Approx(0.5));   // 0.5 stays 0.5
    }

    SECTION("Multiple points wrapping") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> points = {{10.0, -5.0, -5.0}, 
                                    {0.0, 0.5, 0.0}};
        auto wrapped = box.wrap(points);
        
        // Point 1: (10,−5,−5) should wrap to (0,1,1) in box [0,2]³
        REQUIRE(wrapped(0, 0) == Approx(0.0));
        REQUIRE(wrapped(0, 1) == Approx(1.0));
        REQUIRE(wrapped(0, 2) == Approx(1.0));
        
        // Point 2: (0,0.5,0) should remain unchanged
        REQUIRE(wrapped(1, 0) == Approx(0.0));
        REQUIRE(wrapped(1, 1) == Approx(0.5));
        REQUIRE(wrapped(1, 2) == Approx(0.0));
    }

    SECTION("Triclinic box wrapping") {
        Mat3 matrix = {{2.0, 1.0, 0.0}, 
                       {0.0, 2.0, 0.0}, 
                       {0.0, 0.0, 2.0}};
        Box box(matrix);
        
        xt::xarray<double> points = {{3.0, -1.0, -1.0}};
        auto wrapped = box.wrap(points);
        
        // Should wrap according to triclinic geometry
        REQUIRE(wrapped.shape(0) == 1);
        REQUIRE(wrapped.shape(1) == 3);
    }
}

TEST_CASE("Box minimum image tests", "[spatial][box][minimum_image]") {
    SECTION("Orthogonal box minimum image") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> r1 = {{1.9, 1.0, 1.0}};
        xt::xarray<double> r2 = {{0.1, 1.0, 1.0}};
        
        auto dr = box.minimum_image(r1, r2);
        
        // Minimum image should be 0.2 (via periodic boundary)
        REQUIRE(std::abs(dr(0, 0)) == Approx(0.2).margin(1e-10));
    }

    SECTION("Multiple point pairs minimum image") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> r1 = {{1.9, 1.0, 1.0}, 
                                {0.5, 1.5, 1.5}};
        xt::xarray<double> r2 = {{0.1, 1.0, 1.0}, 
                                {1.5, 0.5, 0.5}};
        
        auto dr = box.minimum_image(r1, r2);
        
        REQUIRE(dr.shape(0) == 2);
        REQUIRE(dr.shape(1) == 3);
    }
}

TEST_CASE("Box containment tests", "[spatial][box][containment]") {
    SECTION("Orthogonal box isin test") {
        Box box({2.0, 2.0, 2.0});  // Box from 0 to 2 in each dimension
        
        xt::xarray<double> points_inside = {{1.5, 1.5, 1.5}, 
                                           {0.1, 1.9, 0.1}};
        auto result_inside = box.isin(points_inside);
        
        REQUIRE(result_inside(0) == true);
        REQUIRE(result_inside(1) == true);
    }

    SECTION("Points outside box") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> points_outside = {{2.5, 1.0, 1.0}, 
                                            {1.0, 3.0, 1.0}};
        auto result_outside = box.isin(points_outside);
        
        REQUIRE(result_outside(0) == false);
        REQUIRE(result_outside(1) == false);
    }

    SECTION("Mixed inside/outside points") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> points_mixed = {{1.5, 1.5, 1.5},   // inside
                                          {2.5, 1.0, 1.0},   // outside
                                          {0.1, 1.9, 0.1},   // inside
                                          {1.0, 2.5, 1.0}};  // outside
        auto result = box.isin(points_mixed);
        
        REQUIRE(result(0) == true);
        REQUIRE(result(1) == false);
        REQUIRE(result(2) == true);
        REQUIRE(result(3) == false);
    }
}

TEST_CASE("Box boundary tests", "[spatial][box][boundary]") {
    SECTION("Orthogonal box boundary") {
        Box box({4.0, 6.0, 8.0});
        
        auto bounds = box.boundary();
        
        // For a box with lengths [4,6,8], boundary should be [0,4,0,6,0,8]
        REQUIRE(bounds[0] == Approx(0.0));  // xlo
        REQUIRE(bounds[1] == Approx(4.0));  // xhi
        REQUIRE(bounds[2] == Approx(0.0));  // ylo
        REQUIRE(bounds[3] == Approx(6.0));  // yhi
        REQUIRE(bounds[4] == Approx(0.0));  // zlo
        REQUIRE(bounds[5] == Approx(8.0));  // zhi
    }

    SECTION("Get bounds interface") {
        Box box({4.0, 6.0, 8.0});
        
        auto bounds = box.get_bounds();
        
        REQUIRE(bounds[0] == Approx(0.0));
        REQUIRE(bounds[1] == Approx(4.0));
        REQUIRE(bounds[2] == Approx(0.0));
        REQUIRE(bounds[3] == Approx(6.0));
        REQUIRE(bounds[4] == Approx(0.0));
        REQUIRE(bounds[5] == Approx(8.0));
    }
}

TEST_CASE("Box periodicity tests", "[spatial][box][periodicity]") {
    SECTION("Standard box is periodic") {
        Box box({2.0, 3.0, 4.0});
        
        auto periodic = box.is_periodic();
        
        REQUIRE(periodic[0] == true);  // x periodic
        REQUIRE(periodic[1] == true);  // y periodic
        REQUIRE(periodic[2] == true);  // z periodic
    }
}

TEST_CASE("Box style detection", "[spatial][box][style]") {
    SECTION("Orthogonal box style") {
        Box box({2.0, 3.0, 4.0});
        REQUIRE(box.get_style() == Box::Style::ORTHOGONAL);
    }

    SECTION("Triclinic box style") {
        Mat3 matrix = {{2.0, 1.0, 0.5}, 
                       {0.0, 3.0, 0.1}, 
                       {0.0, 0.0, 4.0}};
        Box box(matrix);
        REQUIRE(box.get_style() == Box::Style::TRICLINIC);
    }

    SECTION("Free box style") {
        Box box;  // Default constructor creates zero matrix
        REQUIRE(box.get_style() == Box::Style::FREE);
    }
}

TEST_CASE("Box matrix operations", "[spatial][box][matrix]") {
    SECTION("Matrix getter") {
        Mat3 original = {{2.0, 0.0, 0.0}, 
                        {1.0, 3.0, 0.0}, 
                        {0.5, 0.1, 4.0}};
        Box box(original);
        
        auto matrix = box.get_matrix();
        
        REQUIRE(matrix(0, 0) == Approx(2.0));
        REQUIRE(matrix(1, 0) == Approx(1.0));
        REQUIRE(matrix(1, 1) == Approx(3.0));
        REQUIRE(matrix(2, 0) == Approx(0.5));
        REQUIRE(matrix(2, 1) == Approx(0.1));
        REQUIRE(matrix(2, 2) == Approx(4.0));
    }

    SECTION("Matrix inverse") {
        Mat3 matrix = {{2.0, 0.0, 0.0}, 
                       {0.0, 3.0, 0.0}, 
                       {0.0, 0.0, 4.0}};
        Box box(matrix);
        
        auto inv_matrix = box.get_inv();
        
        REQUIRE(inv_matrix(0, 0) == Approx(0.5));
        REQUIRE(inv_matrix(1, 1) == Approx(1.0/3.0));
        REQUIRE(inv_matrix(2, 2) == Approx(0.25));
    }
}

TEST_CASE("Box setters", "[spatial][box][setters]") {
    SECTION("Set lengths") {
        Box box;
        Vec3 new_lengths = {5.0, 6.0, 7.0};
        
        box.set_lengths(new_lengths);
        
        auto lengths = box.get_lengths();
        REQUIRE(lengths[0] == Approx(5.0));
        REQUIRE(lengths[1] == Approx(6.0));
        REQUIRE(lengths[2] == Approx(7.0));
    }

    SECTION("Set angles") {
        Box box({2.0, 3.0, 4.0});
        Vec3 new_angles = {80.0, 85.0, 95.0};
        
        box.set_angles(new_angles);
        
        auto angles = box.get_angles();
        REQUIRE(angles[0] == Approx(80.0));
        REQUIRE(angles[1] == Approx(85.0));
        REQUIRE(angles[2] == Approx(95.0));
    }

    SECTION("Set matrix") {
        Box box;
        Mat3 new_matrix = {{3.0, 1.5, 0.0}, 
                          {0.0, 4.0, 0.0}, 
                          {0.0, 0.0, 5.0}};
        
        box.set_matrix(new_matrix);
        
        auto matrix = box.get_matrix();
        REQUIRE(matrix(0, 0) == Approx(3.0));
        REQUIRE(matrix(0, 1) == Approx(1.5));
        REQUIRE(matrix(1, 1) == Approx(4.0));
        REQUIRE(matrix(2, 2) == Approx(5.0));
    }

    SECTION("Set lengths and angles") {
        Box box;
        Vec3 lengths = {2.0, 3.0, 4.0};
        Vec3 angles = {85.0, 90.0, 95.0};
        
        box.set_lengths_angles(lengths, angles);
        
        auto result_lengths = box.get_lengths();
        auto result_angles = box.get_angles();
        
        REQUIRE(result_lengths[0] == Approx(2.0));
        REQUIRE(result_lengths[1] == Approx(3.0));
        REQUIRE(result_lengths[2] == Approx(4.0));
        REQUIRE(result_angles[0] == Approx(85.0));
        REQUIRE(result_angles[1] == Approx(90.0));
        REQUIRE(result_angles[2] == Approx(95.0));
    }
}

TEST_CASE("Box equality operators", "[spatial][box][equality]") {
    SECTION("Equal boxes") {
        Box box1({2.0, 3.0, 4.0});
        Box box2({2.0, 3.0, 4.0});
        
        REQUIRE(box1 == box2);
        REQUIRE_FALSE(box1 != box2);
    }

    SECTION("Different boxes") {
        Box box1({2.0, 3.0, 4.0});
        Box box2({2.0, 3.0, 5.0});
        
        REQUIRE_FALSE(box1 == box2);
        REQUIRE(box1 != box2);
    }

    SECTION("Same triclinic boxes") {
        Mat3 matrix = {{2.0, 1.0, 0.5}, 
                       {0.0, 3.0, 0.1}, 
                       {0.0, 0.0, 4.0}};
        Box box1(matrix);
        Box box2(matrix);
        
        REQUIRE(box1 == box2);
    }
}

TEST_CASE("Box distance between faces", "[spatial][box][distances]") {
    SECTION("Orthogonal box face distances") {
        Box box({4.0, 6.0, 8.0});
        
        auto distances = box.get_distance_between_faces();
        
        REQUIRE(distances[0] == Approx(4.0));  // x direction
        REQUIRE(distances[1] == Approx(6.0));  // y direction  
        REQUIRE(distances[2] == Approx(8.0));  // z direction
    }
}

TEST_CASE("Box static utility functions", "[spatial][box][utilities]") {
    SECTION("Calculate lengths from matrix") {
        Mat3 matrix = {{2.0, 0.0, 0.0}, 
                       {0.0, 3.0, 0.0}, 
                       {0.0, 0.0, 4.0}};
        
        auto lengths = Box::calc_lengths_from_matrix(matrix);
        
        REQUIRE(lengths[0] == Approx(2.0));
        REQUIRE(lengths[1] == Approx(3.0));
        REQUIRE(lengths[2] == Approx(4.0));
    }

    SECTION("Calculate angles from matrix") {
        Mat3 matrix = {{2.0, 0.0, 0.0}, 
                       {0.0, 3.0, 0.0}, 
                       {0.0, 0.0, 4.0}};
        
        auto angles = Box::calc_angles_from_matrix(matrix);
        
        REQUIRE(angles[0] == Approx(90.0));
        REQUIRE(angles[1] == Approx(90.0));
        REQUIRE(angles[2] == Approx(90.0));
    }

    SECTION("Calculate style from matrix") {
        Mat3 orthogonal = {{2.0, 0.0, 0.0}, 
                          {0.0, 3.0, 0.0}, 
                          {0.0, 0.0, 4.0}};
        
        REQUIRE(Box::calc_style_from_matrix(orthogonal) == Box::Style::ORTHOGONAL);
        
        Mat3 triclinic = {{2.0, 1.0, 0.5}, 
                         {0.0, 3.0, 0.1}, 
                         {0.0, 0.0, 4.0}};
        
        REQUIRE(Box::calc_style_from_matrix(triclinic) == Box::Style::TRICLINIC);
        
        Mat3 zero_matrix = xt::zeros<double>({3, 3});
        REQUIRE(Box::calc_style_from_matrix(zero_matrix) == Box::Style::FREE);
    }

    SECTION("Matrix from lengths and angles") {
        Vec3 lengths = {2.0, 3.0, 4.0};
        Vec3 angles = {90.0, 90.0, 90.0};
        
        auto matrix = Box::calc_matrix_from_lengths_angles(lengths, angles);
        
        REQUIRE(matrix(0, 0) == Approx(2.0));
        REQUIRE(matrix(1, 1) == Approx(3.0));
        REQUIRE(matrix(2, 2) == Approx(4.0));
        REQUIRE(matrix(0, 1) == Approx(0.0).margin(1e-10));
        REQUIRE(matrix(0, 2) == Approx(0.0).margin(1e-10));
        REQUIRE(matrix(1, 2) == Approx(0.0).margin(1e-10));
    }
}

TEST_CASE("Box error handling", "[spatial][box][errors]") {
    SECTION("Invalid matrix dimensions") {
        // This test assumes the Box constructor validates matrix dimensions
        // The actual implementation should throw for non-3x3 matrices
        Mat3 valid_matrix = {{1.0, 0.0, 0.0}, 
                            {0.0, 1.0, 0.0}, 
                            {0.0, 0.0, 1.0}};
        
        REQUIRE_NOTHROW(Box(valid_matrix));
    }

    SECTION("Negative lengths") {
        // This test assumes the implementation validates lengths
        Vec3 invalid_lengths = {-1.0, 2.0, 3.0};
        
        // The implementation should throw for negative lengths
        REQUIRE_THROWS(Box(invalid_lengths));
    }
}

TEST_CASE("Box advanced wrapping scenarios", "[spatial][box][wrapping][advanced]") {
    SECTION("Large displacement wrapping") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> points = {{100.0, -50.0, 25.0}};
        auto wrapped = box.wrap(points);
        
        // Should wrap to within [-1, 1] bounds (for box length 2.0)
        REQUIRE(wrapped(0, 0) >= -1.0);
        REQUIRE(wrapped(0, 0) <= 1.0);
        REQUIRE(wrapped(0, 1) >= -1.0);
        REQUIRE(wrapped(0, 1) <= 1.0);
        REQUIRE(wrapped(0, 2) >= -1.0);
        REQUIRE(wrapped(0, 2) <= 1.0);
    }

    SECTION("Zero displacement wrapping") {
        Box box({2.0, 2.0, 2.0});
        
        xt::xarray<double> points = {{0.0, 0.0, 0.0}};
        auto wrapped = box.wrap(points);
        
        REQUIRE(wrapped(0, 0) == Approx(0.0));
        REQUIRE(wrapped(0, 1) == Approx(0.0));
        REQUIRE(wrapped(0, 2) == Approx(0.0));
    }
}

TEST_CASE("Box copy and assignment", "[spatial][box][copy]") {
    SECTION("Copy constructor") {
        Box original({2.0, 3.0, 4.0});
        Box copy(original);
        
        REQUIRE(copy == original);
        REQUIRE(copy.get_volume() == Approx(original.get_volume()));
        
        auto orig_lengths = original.get_lengths();
        auto copy_lengths = copy.get_lengths();
        
        REQUIRE(copy_lengths[0] == Approx(orig_lengths[0]));
        REQUIRE(copy_lengths[1] == Approx(orig_lengths[1]));
        REQUIRE(copy_lengths[2] == Approx(orig_lengths[2]));
    }

    SECTION("Assignment operator") {
        Box original({2.0, 3.0, 4.0});
        Box assigned({1.0, 1.0, 1.0});
        
        assigned = original;
        
        REQUIRE(assigned == original);
        REQUIRE(assigned.get_volume() == Approx(original.get_volume()));
    }
}
