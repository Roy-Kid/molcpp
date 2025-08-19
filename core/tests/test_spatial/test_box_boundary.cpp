#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/spatial/box.hpp"
#include "molcpp/spatial/boundary.hpp"
#include <xtensor/containers/xarray.hpp>

using namespace molcpp;
using namespace Catch;

TEST_CASE("Box construction and factories", "[spatial]") {
    
    SECTION("Box construction") {
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        // Create a simple cubic matrix
        Mat3<float> matrix = xt::zeros<float>({3, 3});
        matrix(0, 0) = 10.0f;
        matrix(1, 1) = 10.0f;
        matrix(2, 2) = 10.0f;
        
        Box box(matrix, origin, pbc);
        
        REQUIRE(box.matrix()(0, 0) == Approx(10.0f));
        REQUIRE(box.matrix()(1, 1) == Approx(10.0f));
        REQUIRE(box.matrix()(2, 2) == Approx(10.0f));
        REQUIRE(box.origin()(0) == Approx(0.0f));
        REQUIRE(box.origin()(1) == Approx(0.0f));
        REQUIRE(box.origin()(2) == Approx(0.0f));
        REQUIRE(box.pbc()(0) == true);
        REQUIRE(box.pbc()(1) == true);
        REQUIRE(box.pbc()(2) == true);
    }
    
    SECTION("Cube factory") {
        Vec3<float> origin = {1.0f, 2.0f, 3.0f};
        Vec3<bool> pbc = {true, false, true};
        
        Box box = Box::cube(5.0f, origin, pbc);
        
        REQUIRE(box.matrix()(0, 0) == Approx(5.0f));
        REQUIRE(box.matrix()(1, 1) == Approx(5.0f));
        REQUIRE(box.matrix()(2, 2) == Approx(5.0f));
        REQUIRE(box.origin()(0) == Approx(1.0f));
        REQUIRE(box.origin()(1) == Approx(2.0f));
        REQUIRE(box.origin()(2) == Approx(3.0f));
        REQUIRE(box.pbc()(0) == true);
        REQUIRE(box.pbc()(1) == false);
        REQUIRE(box.pbc()(2) == true);
    }
    
    SECTION("Orthorhombic factory") {
        Vec3<float> lengths = {6.0f, 8.0f, 10.0f};
        Vec3<float> origin = {-1.0f, -2.0f, -3.0f};
        Vec3<bool> pbc = {false, false, false};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        REQUIRE(box.matrix()(0, 0) == Approx(6.0f));
        REQUIRE(box.matrix()(1, 1) == Approx(8.0f));
        REQUIRE(box.matrix()(2, 2) == Approx(10.0f));
        REQUIRE(box.origin()(0) == Approx(-1.0f));
        REQUIRE(box.origin()(1) == Approx(-2.0f));
        REQUIRE(box.origin()(2) == Approx(-3.0f));
        REQUIRE(box.pbc()(0) == false);
        REQUIRE(box.pbc()(1) == false);
        REQUIRE(box.pbc()(2) == false);
    }
}

TEST_CASE("Box coordinate transforms", "[spatial]") {
    
    SECTION("Orthorhombic coordinate transforms") {
        Vec3<float> lengths = {5.0f, 6.0f, 7.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point transform
        XYZ cart_point = {2.5f, 3.0f, 3.5f};
        XYZ frac_point = box.toFrac(cart_point);
        XYZ back_to_cart = box.toCart(frac_point);
        
        // Should be approximately equal (within float precision)
        REQUIRE(xt::allclose(cart_point, back_to_cart, 1e-6f));
        
        // Test batch transform
        XYZ cart_batch = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
        XYZ frac_batch = box.toFrac(cart_batch);
        XYZ back_to_cart_batch = box.toCart(frac_batch);
        
        REQUIRE(xt::allclose(cart_batch, back_to_cart_batch, 1e-6f));
    }
    
    SECTION("Triclinic coordinate transforms") {
        // Create a skewed matrix
        Mat3<float> matrix = xt::zeros<float>({3, 3});
        matrix(0, 0) = 10.0f;
        matrix(1, 0) = 2.0f;  // skewed
        matrix(1, 1) = 8.0f;
        matrix(2, 2) = 12.0f;
        
        Vec3<float> origin = {1.0f, 2.0f, 3.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box(matrix, origin, pbc);
        
        // Test single point
        XYZ cart_single = {5.0f, 6.0f, 7.0f};
        XYZ frac_single = box.toFrac(cart_single);
        XYZ cart_back_single = box.toCart(frac_single);
        
        REQUIRE(xt::allclose(cart_single, cart_back_single, 1e-6f));
        
        // Test batch
        XYZ cart_batch = {{5.0f, 6.0f, 7.0f}, {8.0f, 9.0f, 10.0f}};
        XYZ frac_batch = box.toFrac(cart_batch);
        XYZ cart_back_batch = box.toCart(frac_batch);
        
        REQUIRE(xt::allclose(cart_batch, cart_back_batch, 1e-6f));
    }
}

TEST_CASE("Box volume calculation", "[spatial]") {
    
    SECTION("Orthorhombic volume") {
        Vec3<float> lengths = {5.0f, 6.0f, 7.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        REQUIRE(box.getVolume() == Approx(5.0f * 6.0f * 7.0f));
    }
    
    SECTION("Triclinic volume") {
        Mat3<float> matrix = xt::zeros<float>({3, 3});
        matrix(0, 0) = 10.0f;
        matrix(1, 0) = 2.0f;  // skewed
        matrix(1, 1) = 8.0f;
        matrix(2, 2) = 12.0f;
        
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box(matrix, origin, pbc);
        
        float expected_volume = std::abs(10.0f * 8.0f * 12.0f);  // |det(H)|
        REQUIRE(box.getVolume() == Approx(expected_volume));
    }
}

TEST_CASE("Box membership testing", "[spatial]") {
    
    SECTION("Points inside orthorhombic box") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {false, false, false};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point inside
        XYZ point_inside = {5.0f, 5.0f, 5.0f};
        auto result_inside = box.isIn(point_inside);
        REQUIRE(result_inside());
        
        // Test single point outside
        XYZ point_outside = {15.0f, 15.0f, 15.0f};
        auto result_outside = box.isIn(point_outside);
        REQUIRE(!result_outside());
        
        // Test batch
        XYZ points_batch = {{5.0f, 5.0f, 5.0f}, {15.0f, 15.0f, 15.0f}};
        auto result_batch = box.isIn(points_batch);
        
        REQUIRE(result_batch(0) == true);
        REQUIRE(result_batch(1) == false);
    }
}
