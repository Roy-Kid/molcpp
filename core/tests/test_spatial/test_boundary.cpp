#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/spatial/box.hpp"
#include "molcpp/spatial/boundary.hpp"
#include <xtensor/containers/xarray.hpp>

using namespace molcpp;
using namespace Catch;

TEST_CASE("OpenBoundary behavior", "[spatial][boundary]") {
    
    SECTION("Open boundary wrapping") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {false, false, false};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point wrapping (should return unchanged)
        XYZ point = {15.0f, -5.0f, 25.0f};
        XYZ wrapped = box.wrap(point);
        REQUIRE(xt::allclose(point, wrapped));
        
        // Test batch wrapping
        XYZ points = {{5.0f, 5.0f, 5.0f}, {15.0f, 15.0f, 15.0f}};
        XYZ wrapped_batch = box.wrap(points);
        REQUIRE(xt::allclose(points, wrapped_batch));
    }
    
    SECTION("Open boundary delta") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {false, false, false};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point delta (should return b - a)
        XYZ a = {1.0f, 2.0f, 3.0f};
        XYZ b = {4.0f, 6.0f, 9.0f};
        XYZ delta = box.delta(a, b, false);
        XYZ expected = {3.0f, 4.0f, 6.0f};
        REQUIRE(xt::allclose(delta, expected));
        
        // Test batch delta
        XYZ a_batch = {{1.0f, 2.0f, 3.0f}, {2.0f, 3.0f, 4.0f}};
        XYZ b_batch = {{4.0f, 6.0f, 9.0f}, {5.0f, 7.0f, 10.0f}};
        XYZ delta_batch = box.delta(a_batch, b_batch, false);
        XYZ expected_batch = {{3.0f, 4.0f, 6.0f}, {3.0f, 4.0f, 6.0f}};
        REQUIRE(xt::allclose(delta_batch, expected_batch));
    }
    
    SECTION("Open boundary inside test") {
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

TEST_CASE("PeriodicBoundary behavior", "[spatial][boundary]") {
    
    SECTION("Periodic boundary wrapping") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point wrapping
        XYZ point = {15.0f, -5.0f, 25.0f};
        XYZ wrapped = box.wrap(point);
        
        // Should wrap to [0, 10) range
        REQUIRE(wrapped(0) == Approx(5.0f));
        REQUIRE(wrapped(1) == Approx(5.0f));
        REQUIRE(wrapped(2) == Approx(5.0f));
        
        // Test batch wrapping
        XYZ points = {{15.0f, -5.0f, 25.0f}, {35.0f, 45.0f, -15.0f}};
        XYZ wrapped_batch = box.wrap(points);
        
        REQUIRE(wrapped_batch(0, 0) == Approx(5.0f));
        REQUIRE(wrapped_batch(0, 1) == Approx(5.0f));
        REQUIRE(wrapped_batch(0, 2) == Approx(5.0f));
        REQUIRE(wrapped_batch(1, 0) == Approx(5.0f));
        REQUIRE(wrapped_batch(1, 1) == Approx(5.0f));
        REQUIRE(wrapped_batch(1, 2) == Approx(5.0f));
    }
    
    SECTION("Periodic boundary minimum image convention") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, true, true};
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test single point MIC
        XYZ a_single = {1.0f, 1.0f, 1.0f};
        XYZ b_single = {9.0f, 9.0f, 9.0f};
        
        XYZ delta_single = box.delta(a_single, b_single, true);
        
        // Should wrap to shortest distance
        REQUIRE(delta_single(0) == Approx(-2.0f));
        REQUIRE(delta_single(1) == Approx(-2.0f));
        REQUIRE(delta_single(2) == Approx(-2.0f));
        
        // Test batch MIC
        XYZ a_batch = {{1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f}};
        XYZ b_batch = {{9.0f, 9.0f, 9.0f}, {8.0f, 8.0f, 8.0f}};
        
        XYZ delta_batch = box.delta(a_batch, b_batch, true);
        
        REQUIRE(delta_batch(0, 0) == Approx(-2.0f));
        REQUIRE(delta_batch(0, 1) == Approx(-2.0f));
        REQUIRE(delta_batch(0, 2) == Approx(-2.0f));
        REQUIRE(delta_batch(1, 0) == Approx(-4.0f));
        REQUIRE(delta_batch(1, 1) == Approx(-4.0f));
        REQUIRE(delta_batch(1, 2) == Approx(-4.0f));
    }
    
    SECTION("Hybrid periodic boundary") {
        Vec3<float> lengths = {10.0f, 10.0f, 10.0f};
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        Vec3<bool> pbc = {true, false, true};  // x and z periodic, y not
        
        Box box = Box::orthorhombic(lengths, origin, pbc);
        
        // Test wrapping - only x and z should wrap
        XYZ point = {15.0f, 15.0f, 25.0f};
        XYZ wrapped = box.wrap(point);
        
        REQUIRE(wrapped(0) == Approx(5.0f));   // x wrapped
        REQUIRE(wrapped(1) == Approx(15.0f));  // y unchanged
        REQUIRE(wrapped(2) == Approx(5.0f));   // z wrapped
        
        // Test MIC - only x and z should apply MIC
        XYZ a = {1.0f, 1.0f, 1.0f};
        XYZ b = {9.0f, 11.0f, 9.0f};
        
        XYZ delta = box.delta(a, b, true);
        
        REQUIRE(delta(0) == Approx(-2.0f));   // x MIC applied
        REQUIRE(delta(1) == Approx(10.0f));   // y no MIC
        REQUIRE(delta(2) == Approx(-2.0f));   // z MIC applied
    }
}
