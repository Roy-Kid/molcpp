#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/spatial/region.hpp"
#include "molcpp/spatial/boundary.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/generators/xbuilder.hpp>
#include <xtensor/generators/xrandom.hpp>
#include <memory>
#include <vector>
#include <cmath>

using namespace molcpp;
using Catch::Approx;

TEST_CASE("InsideCube region tests", "[spatial][region][cube]") {
    SECTION("Cube constructor") {
        Vec3 lo = {0.0, 0.0, 0.0};
        double L = 2.0;
        InsideCube cube(lo, L);
        
        auto boundary = cube.boundary();
        REQUIRE(boundary[0] == Approx(0.0));  // xlo
        REQUIRE(boundary[1] == Approx(2.0));  // xhi
        REQUIRE(boundary[2] == Approx(0.0));  // ylo
        REQUIRE(boundary[3] == Approx(2.0));  // yhi
        REQUIRE(boundary[4] == Approx(0.0));  // zlo
        REQUIRE(boundary[5] == Approx(2.0));  // zhi
        
        REQUIRE(cube.volume() == Approx(8.0));
    }

    SECTION("Box constructor") {
        Vec3 lo = {0.0, 0.0, 0.0};
        Vec3 lengths = {2.0, 3.0, 4.0};
        InsideCube box(lo, lengths);
        
        auto boundary = box.boundary();
        REQUIRE(boundary[0] == Approx(0.0));  // xlo
        REQUIRE(boundary[1] == Approx(2.0));  // xhi
        REQUIRE(boundary[2] == Approx(0.0));  // ylo
        REQUIRE(boundary[3] == Approx(3.0));  // yhi
        REQUIRE(boundary[4] == Approx(0.0));  // zlo
        REQUIRE(boundary[5] == Approx(4.0));  // zhi
        
        REQUIRE(box.volume() == Approx(24.0));
    }

    SECTION("Point inside and outside tests") {
        InsideCube cube({0.0, 0.0, 0.0}, 2.0);

        xt::xarray<double> coords_inside = {{1.0, 1.0, 1.0}};
        REQUIRE(cube.isin(coords_inside) == true);

        xt::xarray<double> coords_outside = {{3.0, 1.0, 1.0}};
        REQUIRE(cube.isin(coords_outside) == false);

        // Test mask function
        xt::xarray<double> coords_mixed = {
            {0.5, 0.5, 0.5},  // inside
            {1.0, 1.0, 1.0},  // inside
            {2.5, 1.5, 1.5}   // outside
        };
        
        auto mask = cube.mask(coords_mixed);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == true);
        REQUIRE(mask(2) == false);
    }
}

TEST_CASE("InsideSphere region tests", "[spatial][region][sphere]") {
    Vec3 center = {0.0, 0.0, 0.0};
    double R = 1.0;
    InsideSphere sphere(center, R);

    SECTION("Volume calculation") {
        double expected_volume = 4.0 * M_PI / 3.0; // 4π/3 * r³
        REQUIRE(sphere.volume() == Approx(expected_volume));
    }

    SECTION("Point containment") {
        xt::xarray<double> coords_inside = {{0.5, 0.0, 0.0}};
        REQUIRE(sphere.isin(coords_inside) == true);

        xt::xarray<double> coords_outside = {{2.0, 0.0, 0.0}};
        REQUIRE(sphere.isin(coords_outside) == false);

        xt::xarray<double> coords_surface = {{1.0, 0.0, 0.0}};
        REQUIRE(sphere.isin(coords_surface) == true);
    }

    SECTION("Mask function") {
        xt::xarray<double> coords = {
            {0.0, 0.0, 0.0},    // center - inside
            {0.8, 0.6, 0.0},    // inside (0.8² + 0.6² = 1.0)
            {1.5, 0.0, 0.0}     // outside
        };
        
        auto mask = sphere.mask(coords);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == true);
        REQUIRE(mask(2) == false);
    }
}

TEST_CASE("InsideCylinder region tests", "[spatial][region][cylinder]") {
    Vec3 center1 = {0.0, 0.0, 0.0};
    Vec3 center2 = {0.0, 0.0, 2.0};
    double radius = 1.0;
    InsideCylinder cylinder(center1, center2, radius);

    SECTION("Basic properties") {
        REQUIRE(cylinder.get_height() == Approx(2.0));
        REQUIRE(cylinder.get_radius() == Approx(1.0));
        
        double expected_volume = M_PI * radius * radius * 2.0;
        REQUIRE(cylinder.volume() == Approx(expected_volume));
    }

    SECTION("Point containment") {
        // Point inside cylinder
        xt::xarray<double> coords_inside = {{0.5, 0.0, 1.0}};
        REQUIRE(cylinder.isin(coords_inside) == true);

        // Point outside radius
        xt::xarray<double> coords_outside_radius = {{1.5, 0.0, 1.0}};
        REQUIRE(cylinder.isin(coords_outside_radius) == false);

        // Point outside height
        xt::xarray<double> coords_outside_height = {{0.5, 0.0, 3.0}};
        REQUIRE(cylinder.isin(coords_outside_height) == false);
    }

    SECTION("Mask function") {
        xt::xarray<double> coords = {
            {0.0, 0.0, 1.0},    // center - inside
            {0.9, 0.0, 1.0},    // inside
            {1.1, 0.0, 1.0},    // outside radius
            {0.5, 0.0, 2.5}     // outside height
        };
        
        auto mask = cylinder.mask(coords);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == true);
        REQUIRE(mask(2) == false);
        REQUIRE(mask(3) == false);
    }
}

TEST_CASE("NearPlane region tests", "[spatial][region][plane]") {
    Vec3 point = {0.0, 0.0, 0.0};
    Vec3 normal = {0.0, 0.0, 1.0};  // XY plane
    double thickness = 0.5;
    NearPlane plane(point, normal, thickness);

    SECTION("Point containment") {
        // Point in plane
        xt::xarray<double> coords_in = {{1.0, 1.0, 0.0}};
        REQUIRE(plane.isin(coords_in) == true);

        // Point within thickness
        xt::xarray<double> coords_near = {{1.0, 1.0, 0.4}};
        REQUIRE(plane.isin(coords_near) == true);

        // Point outside thickness
        xt::xarray<double> coords_far = {{1.0, 1.0, 0.6}};
        REQUIRE(plane.isin(coords_far) == false);
    }

    SECTION("Mask function") {
        xt::xarray<double> coords = {
            {0.0, 0.0, 0.0},    // on plane
            {1.0, 1.0, 0.3},    // within thickness
            {0.0, 0.0, -0.4},   // within thickness (negative side)
            {0.0, 0.0, 0.7}     // outside thickness
        };
        
        auto mask = plane.mask(coords);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == true);
        REQUIRE(mask(2) == true);
        REQUIRE(mask(3) == false);
    }
}

TEST_CASE("Boolean region combinations", "[spatial][region][boolean]") {
    auto cube = std::make_shared<InsideCube>(Vec3{-1.0, -1.0, -1.0}, 2.0);
    auto sphere = std::make_shared<InsideSphere>(Vec3{0.0, 0.0, 0.0}, 1.0);

    SECTION("AndRegion tests") {
        AndRegion intersection(cube, sphere);

        // Point inside both
        xt::xarray<double> coords_inside = {{0.5, 0.0, 0.0}};
        REQUIRE(intersection.isin(coords_inside) == true);

        // Point in cube but not sphere
        xt::xarray<double> coords_cube_only = {{0.9, 0.9, 0.9}};
        REQUIRE(intersection.isin(coords_cube_only) == false);

        // Test mask
        xt::xarray<double> coords = {
            {0.5, 0.0, 0.0},    // in both
            {0.9, 0.9, 0.9},    // cube only
            {2.0, 0.0, 0.0}     // neither
        };
        
        auto mask = intersection.mask(coords);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == false);
        REQUIRE(mask(2) == false);
    }

    SECTION("OrRegion tests") {
        auto sphere1 = std::make_shared<InsideSphere>(Vec3{-2.0, 0.0, 0.0}, 1.0);
        auto sphere2 = std::make_shared<InsideSphere>(Vec3{2.0, 0.0, 0.0}, 1.0);
        OrRegion union_region(sphere1, sphere2);

        // Point in first sphere
        xt::xarray<double> coords_sphere1 = {{-1.5, 0.0, 0.0}};
        REQUIRE(union_region.isin(coords_sphere1) == true);

        // Point in second sphere
        xt::xarray<double> coords_sphere2 = {{1.5, 0.0, 0.0}};
        REQUIRE(union_region.isin(coords_sphere2) == true);

        // Point in neither
        xt::xarray<double> coords_neither = {{0.0, 0.0, 0.0}};
        REQUIRE(union_region.isin(coords_neither) == false);

        // Test mask
        xt::xarray<double> coords = {
            {-2.0, 0.0, 0.0},   // sphere1 center
            {2.0, 0.0, 0.0},    // sphere2 center
            {0.0, 0.0, 0.0}     // between spheres
        };
        
        auto mask = union_region.mask(coords);
        REQUIRE(mask(0) == true);
        REQUIRE(mask(1) == true);
        REQUIRE(mask(2) == false);
    }

    SECTION("NotRegion tests") {
        NotRegion complement(sphere);

        // Point inside original sphere should be outside NOT region
        xt::xarray<double> coords_inside = {{0.5, 0.0, 0.0}};
        REQUIRE(complement.isin(coords_inside) == false);

        // Point outside original sphere should be inside NOT region
        xt::xarray<double> coords_outside = {{2.0, 0.0, 0.0}};
        REQUIRE(complement.isin(coords_outside) == true);

        // Test mask
        xt::xarray<double> coords = {
            {0.0, 0.0, 0.0},    // sphere center
            {2.0, 0.0, 0.0}     // outside sphere
        };
        
        auto mask = complement.mask(coords);
        REQUIRE(mask(0) == false);
        REQUIRE(mask(1) == true);
    }
}


TEST_CASE("Performance and numerical stability", "[spatial][performance]") {
    SECTION("Large coordinate arrays") {
        InsideSphere sphere({0.0, 0.0, 0.0}, 1.0);
        
        // Create large array of random points
        const int n_points = 1000;
        xt::xarray<double> coords = xt::random::randn<double>({n_points, 3});
        
        // Should not crash and should complete quickly
        auto mask = sphere.mask(coords);
        REQUIRE(mask.size() == n_points);
        
        // Count points inside sphere (approximately π/6 ≈ 0.52 for unit cube in unit sphere)
        int count_inside = 0;
        for (int i = 0; i < n_points; ++i) {
            if (mask(i)) count_inside++;
        }
        
        // Should have some points inside and some outside for random data
        REQUIRE(count_inside > 0);
        REQUIRE(count_inside < n_points);
    }

    SECTION("Numerical tolerance near boundaries") {
        InsideCube cube({0.0, 0.0, 0.0}, 1.0);
        
        // Points very close to boundary
        xt::xarray<double> coords_boundary = {
            {1e-7, 0.5, 0.5},           // Just inside with tolerance
            {1.0 + 1e-7, 0.5, 0.5},     // Just inside with tolerance
            {-1e-5, 0.5, 0.5},          // Just outside tolerance
            {1.0 + 1e-5, 0.5, 0.5}      // Just outside tolerance
        };
        
        auto mask = cube.mask(coords_boundary);
        REQUIRE(mask(0) == true);   // Within tolerance
        REQUIRE(mask(1) == true);   // Within tolerance
        REQUIRE(mask(2) == false);  // Outside tolerance
        REQUIRE(mask(3) == false);  // Outside tolerance
    }
}
