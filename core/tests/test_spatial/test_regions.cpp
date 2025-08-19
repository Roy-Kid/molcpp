#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <memory>

using namespace molcpp;
using namespace Catch;

TEST_CASE("ParallelepipedRegion construction and properties", "[spatial][region]") {
    
    SECTION("Basic construction") {
        Mat3<float> matrix = xt::zeros<float>({3, 3});
        matrix(0, 0) = 10.0f;
        matrix(1, 1) = 8.0f;
        matrix(2, 2) = 12.0f;
        
        Vec3<float> origin = {1.0f, 2.0f, 3.0f};
        
        ParallelepipedRegion region(matrix, origin);
        
        REQUIRE(region.getMatrix()(0, 0) == Approx(10.0f));
        REQUIRE(region.getMatrix()(1, 1) == Approx(8.0f));
        REQUIRE(region.getMatrix()(2, 2) == Approx(12.0f));
        REQUIRE(region.getOrigin()(0) == Approx(1.0f));
        REQUIRE(region.getOrigin()(1) == Approx(2.0f));
        REQUIRE(region.getOrigin()(2) == Approx(3.0f));
    }
    
    SECTION("Volume calculation") {
        // Orthorhombic case
        Mat3<float> matrix_ortho = xt::zeros<float>({3, 3});
        matrix_ortho(0, 0) = 5.0f;
        matrix_ortho(1, 1) = 6.0f;
        matrix_ortho(2, 2) = 7.0f;
        
        Vec3<float> origin = {0.0f, 0.0f, 0.0f};
        
        ParallelepipedRegion region_ortho(matrix_ortho, origin);
        REQUIRE(region_ortho.getVolume() == Approx(5.0f * 6.0f * 7.0f));
        
        // Triclinic case
        Mat3<float> matrix_tric = xt::zeros<float>({3, 3});
        matrix_tric(0, 0) = 10.0f;
        matrix_tric(1, 0) = 2.0f;  // skewed
        matrix_tric(1, 1) = 8.0f;
        matrix_tric(2, 2) = 12.0f;
        
        ParallelepipedRegion region_tric(matrix_tric, origin);
        float expected_volume = std::abs(10.0f * 8.0f * 12.0f);  // |det(H)|
        REQUIRE(region_tric.getVolume() == Approx(expected_volume));
    }
}

TEST_CASE("CubeRegion functionality", "[spatial][region]") {
    
    SECTION("Construction") {
        Vec3<float> lower_corner = {0.0f, 0.0f, 0.0f};
        float edge_length = 10.0f;
        
        CubeRegion region(lower_corner, edge_length);
        
        REQUIRE(region.getLowerCorner()(0) == Approx(0.0f));
        REQUIRE(region.getLowerCorner()(1) == Approx(0.0f));
        REQUIRE(region.getLowerCorner()(2) == Approx(0.0f));
        REQUIRE(region.getEdgeLengths()(0) == Approx(10.0f));
        REQUIRE(region.getEdgeLengths()(1) == Approx(10.0f));
        REQUIRE(region.getEdgeLengths()(2) == Approx(10.0f));
    }
    
    SECTION("Membership testing") {
        Vec3<float> lower_corner = {1.0f, 2.0f, 3.0f};
        float edge_length = 5.0f;
        
        CubeRegion region(lower_corner, edge_length);
        
        // Test point inside
        XYZ point_inside = {3.0f, 4.0f, 5.0f};
        auto result_inside = region.isIn(point_inside);
        REQUIRE(result_inside());
        
        // Test point outside
        XYZ point_outside = {10.0f, 10.0f, 10.0f};
        auto result_outside = region.isIn(point_outside);
        REQUIRE(!result_outside());
        
        // Test batch
        XYZ points_batch = {{3.0f, 4.0f, 5.0f}, {10.0f, 10.0f, 10.0f}};
        auto result_batch = region.isIn(points_batch);
        
        REQUIRE(result_batch(0) == true);
        REQUIRE(result_batch(1) == false);
    }
    
    SECTION("Volume calculation") {
        Vec3<float> lower_corner = {0.0f, 0.0f, 0.0f};
        float edge_length = 5.0f;
        
        CubeRegion region(lower_corner, edge_length);
        REQUIRE(region.getVolume() == Approx(125.0));
    }
}

TEST_CASE("SphereRegion functionality", "[spatial][region]") {
    
    SECTION("Construction") {
        Vec3<float> center = {1.0f, 2.0f, 3.0f};
        float radius = 5.0f;
        
        SphereRegion region(center, radius);
        
        REQUIRE(region.getCenter()(0) == Approx(1.0f));
        REQUIRE(region.getCenter()(1) == Approx(2.0f));
        REQUIRE(region.getCenter()(2) == Approx(3.0f));
        REQUIRE(region.getRadius() == Approx(5.0f));
    }
    
    SECTION("Membership testing") {
        Vec3<float> center = {0.0f, 0.0f, 0.0f};
        float radius = 5.0f;
        
        SphereRegion region(center, radius);
        
        // Test point inside
        XYZ point_inside = {3.0f, 4.0f, 0.0f};
        auto result_inside = region.isIn(point_inside);
        REQUIRE(result_inside());
        
        // Test point outside
        XYZ point_outside = {6.0f, 0.0f, 0.0f};
        auto result_outside = region.isIn(point_outside);
        REQUIRE(!result_outside());
        
        // Test batch
        XYZ points_batch = {{3.0f, 4.0f, 0.0f}, {6.0f, 0.0f, 0.0f}};
        auto result_batch = region.isIn(points_batch);
        
        REQUIRE(result_batch(0) == true);
        REQUIRE(result_batch(1) == false);
    }
    
    SECTION("Volume calculation") {
        Vec3<float> center = {0.0f, 0.0f, 0.0f};
        float radius = 3.0f;
        
        SphereRegion region(center, radius);
        float expected_volume = 4.0f * M_PI * 27.0f / 3.0f;
        REQUIRE(region.getVolume() == Approx(expected_volume));
    }
}

TEST_CASE("CylinderRegion functionality", "[spatial][region]") {
    
    SECTION("Construction") {
        Vec3<float> end1 = {0.0f, 0.0f, 0.0f};
        Vec3<float> end2 = {0.0f, 0.0f, 10.0f};
        float radius = 3.0f;
        
        CylinderRegion region(end1, end2, radius);
        
        REQUIRE(region.getEnd1()(0) == Approx(0.0f));
        REQUIRE(region.getEnd1()(1) == Approx(0.0f));
        REQUIRE(region.getEnd1()(2) == Approx(0.0f));
        REQUIRE(region.getEnd2()(0) == Approx(0.0f));
        REQUIRE(region.getEnd2()(1) == Approx(0.0f));
        REQUIRE(region.getEnd2()(2) == Approx(10.0f));
        REQUIRE(region.getRadius() == Approx(3.0f));
        REQUIRE(region.getHeight() == Approx(10.0f));
    }
    
    SECTION("Membership testing") {
        Vec3<float> end1 = {0.0f, 0.0f, 0.0f};
        Vec3<float> end2 = {0.0f, 0.0f, 10.0f};
        float radius = 3.0f;
        
        CylinderRegion region(end1, end2, radius);
        
        // Test point inside
        XYZ point_inside = {2.0f, 0.0f, 5.0f};
        auto result_inside = region.isIn(point_inside);
        REQUIRE(result_inside());
        
        // Test point outside (too far from axis)
        XYZ point_outside_radius = {4.0f, 0.0f, 5.0f};
        auto result_outside_radius = region.isIn(point_outside_radius);
        REQUIRE(!result_outside_radius());
        
        // Test point outside (beyond height)
        XYZ point_outside_height = {2.0f, 0.0f, 15.0f};
        auto result_outside_height = region.isIn(point_outside_height);
        REQUIRE(!result_outside_height());
    }
    
    SECTION("Volume calculation") {
        Vec3<float> end1 = {0.0f, 0.0f, 0.0f};
        Vec3<float> end2 = {0.0f, 0.0f, 8.0f};
        float radius = 2.0f;
        
        CylinderRegion region(end1, end2, radius);
        float expected_volume = M_PI * 4.0f * 8.0f;
        REQUIRE(region.getVolume() == Approx(expected_volume));
    }
}

TEST_CASE("PlaneRegion functionality", "[spatial][region]") {
    
    SECTION("Construction") {
        Vec3<float> point = {0.0f, 0.0f, 0.0f};
        Vec3<float> normal = {0.0f, 0.0f, 1.0f};
        float thickness = 2.0f;
        
        PlaneRegion region(point, normal, thickness);
        
        REQUIRE(region.getPoint()(0) == Approx(0.0f));
        REQUIRE(region.getPoint()(1) == Approx(0.0f));
        REQUIRE(region.getPoint()(2) == Approx(0.0f));
        REQUIRE(region.getThickness() == Approx(2.0f));
    }
    
    SECTION("Membership testing") {
        Vec3<float> point = {0.0f, 0.0f, 0.0f};
        Vec3<float> normal = {0.0f, 0.0f, 1.0f};
        float thickness = 2.0f;
        
        PlaneRegion region(point, normal, thickness);
        
        // Test point inside (on plane)
        XYZ point_on_plane = {1.0f, 2.0f, 0.0f};
        auto result_on_plane = region.isIn(point_on_plane);
        REQUIRE(result_on_plane());
        
        // Test point inside (within thickness)
        XYZ point_within_thickness = {1.0f, 2.0f, 1.5f};
        auto result_within_thickness = region.isIn(point_within_thickness);
        REQUIRE(result_within_thickness());
        
        // Test point outside (beyond thickness)
        XYZ point_outside = {1.0f, 2.0f, 3.0f};
        auto result_outside = region.isIn(point_outside);
        REQUIRE(!result_outside());
    }
    
    SECTION("Volume calculation") {
        Vec3<float> point = {0.0f, 0.0f, 0.0f};
        Vec3<float> normal = {0.0f, 0.0f, 1.0f};
        float thickness = 2.0f;
        
        PlaneRegion region(point, normal, thickness);
        REQUIRE(std::isinf(region.getVolume()));
    }
}

TEST_CASE("Boolean operation regions", "[spatial][region]") {
    
    SECTION("IntersectionRegion") {
        auto cube1 = std::make_shared<CubeRegion>(Vec3<float>{0.0f, 0.0f, 0.0f}, 10.0f);
        auto cube2 = std::make_shared<CubeRegion>(Vec3<float>{5.0f, 5.0f, 5.0f}, 10.0f);
        
        IntersectionRegion intersection(cube1, cube2);
        
        // Test point in intersection
        XYZ point_in_intersection = {7.0f, 7.0f, 7.0f};
        auto result_in = intersection.isIn(point_in_intersection);
        REQUIRE(result_in());
        
        // Test point not in intersection
        XYZ point_not_in_intersection = {2.0f, 2.0f, 2.0f};
        auto result_not_in = intersection.isIn(point_not_in_intersection);
        REQUIRE(!result_not_in());
    }
    
    SECTION("UnionRegion") {
        auto cube1 = std::make_shared<CubeRegion>(Vec3<float>{0.0f, 0.0f, 0.0f}, 5.0f);
        auto cube2 = std::make_shared<CubeRegion>(Vec3<float>{10.0f, 0.0f, 0.0f}, 5.0f);
        
        UnionRegion union_region(cube1, cube2);
        
        // Test point in first cube
        XYZ point_in_first = {2.0f, 2.0f, 2.0f};
        auto result_in_first = union_region.isIn(point_in_first);
        REQUIRE(result_in_first());
        
        // Test point in second cube
        XYZ point_in_second = {12.0f, 2.0f, 2.0f};
        auto result_in_second = union_region.isIn(point_in_second);
        REQUIRE(result_in_second());
        
        // Test point in neither
        XYZ point_in_neither = {7.0f, 2.0f, 2.0f};
        auto result_in_neither = union_region.isIn(point_in_neither);
        REQUIRE(!result_in_neither());
    }
    
    SECTION("ComplementRegion") {
        auto cube = std::make_shared<CubeRegion>(Vec3<float>{0.0f, 0.0f, 0.0f}, 5.0f);
        
        ComplementRegion complement(cube);
        
        // Test point outside cube (should be in complement)
        XYZ point_outside = {10.0f, 10.0f, 10.0f};
        auto result_outside = complement.isIn(point_outside);
        REQUIRE(result_outside());
        
        // Test point inside cube (should not be in complement)
        XYZ point_inside = {2.0f, 2.0f, 2.0f};
        auto result_inside = complement.isIn(point_inside);
        REQUIRE(!result_inside());
    }
}
