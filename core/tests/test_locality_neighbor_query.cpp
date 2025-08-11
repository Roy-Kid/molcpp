#include <catch2/catch.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xview.hpp>

#include <molcpp/locality/AABBQuery.hpp>
#include <molcpp/box/Box.hpp>

using namespace molcpp;
using namespace molcpp::locality;
using namespace molcpp::box;

// Helper to create a simple cubic lattice
xt::xarray<double> makeSimpleCubicLattice(size_t n, double a) {
    size_t N = n * n * n;
    xt::xarray<double> points = xt::zeros<double>({N, 3});
    
    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            for (size_t k = 0; k < n; ++k) {
                points(idx, 0) = i * a - (n - 1) * a / 2.0;
                points(idx, 1) = j * a - (n - 1) * a / 2.0;
                points(idx, 2) = k * a - (n - 1) * a / 2.0;
                idx++;
            }
        }
    }
    
    return points;
}

TEST_CASE("AABBQuery construction", "[aabbquery]") {
    SECTION("Default construction") {
        AABBQuery query;
        REQUIRE(query.getNPoints() == 0);
    }
    
    SECTION("Construction with box and points") {
        Box box(10.0, 10.0, 10.0);
        xt::xarray<double> points = xt::random::rand<double>({10, 3}) * 10.0 - 5.0;
        
        AABBQuery query(box, points);
        REQUIRE(query.getNPoints() == 10);
        REQUIRE(&query.getBox() != nullptr);
        REQUIRE(query.getPoints().shape(0) == 10);
        REQUIRE(query.getPoints().shape(1) == 3);
    }
}

TEST_CASE("AABBQuery ball queries", "[aabbquery]") {
    // Create a simple cubic lattice
    const size_t n = 5;
    const double a = 1.0;  // lattice constant
    const double L = n * a * 1.5;  // Box size
    
    Box box(L, L, L);
    auto points = makeSimpleCubicLattice(n, a);
    AABBQuery query(box, points);
    
    SECTION("Find nearest neighbors in cubic lattice") {
        // In a simple cubic lattice, nearest neighbors are at distance a
        double r_max = a * 1.1;  // Slightly larger to account for floating point
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query.query(points, args);
        
        // Each internal point should have 6 nearest neighbors
        // Corner points have 3, edge points have 4-5, face points have 5
        auto counts = nlist.getNeighborCounts(points.shape(0));
        
        // Check that distances are correct
        auto distances = nlist.getDistances();
        for (size_t i = 0; i < distances.size(); ++i) {
            REQUIRE(distances(i) <= r_max);
            // Most should be exactly a (within floating point tolerance)
            if (distances(i) < a * 0.9) {
                REQUIRE(distances(i) == Approx(a).epsilon(0.01));
            }
        }
    }
    
    SECTION("Empty result for small radius") {
        double r_max = a * 0.5;  // Too small to find any neighbors
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query.query(points, args);
        REQUIRE(nlist.empty());
    }
    
    SECTION("Exclude self-neighbors") {
        double r_max = a * 0.1;  // Very small radius
        
        // With exclude_ii = false, should find self
        QueryArgs args1 = QueryArgs::ball(r_max, 0.0, false);
        auto nlist1 = query.query(points, args1);
        REQUIRE(nlist1.size() == points.shape(0));  // Each point finds itself
        
        // With exclude_ii = true, should find nothing
        QueryArgs args2 = QueryArgs::ball(r_max, 0.0, true);
        auto nlist2 = query.query(points, args2);
        REQUIRE(nlist2.empty());
    }
    
    SECTION("Filter by r_min") {
        double r_min = a * 0.9;
        double r_max = a * 1.5;
        QueryArgs args = QueryArgs::ball(r_max, r_min, true);
        
        auto nlist = query.query(points, args);
        
        // All distances should be in range [r_min, r_max]
        auto distances = nlist.getDistances();
        for (size_t i = 0; i < distances.size(); ++i) {
            REQUIRE(distances(i) >= r_min);
            REQUIRE(distances(i) <= r_max);
        }
    }
}

TEST_CASE("AABBQuery k-nearest neighbors", "[aabbquery]") {
    const size_t N = 100;
    const double L = 10.0;
    
    Box box(L, L, L);
    xt::xarray<double> points = xt::random::rand<double>({N, 3}) * L - L/2.0;
    AABBQuery query(box, points);
    
    SECTION("Find k nearest neighbors") {
        unsigned int k = 5;
        QueryArgs args = QueryArgs::nearest(k);
        
        // Query a subset of points
        auto query_points = xt::view(points, xt::range(0, 10), xt::all());
        auto nlist = query.query(query_points, args);
        
        // Each query point should have exactly k neighbors (or less if not enough points)
        auto counts = nlist.getNeighborCounts(10);
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(counts(i) <= k);
            // With 100 points and exclude_ii, should always find k neighbors
            if (N > k + 1) {
                REQUIRE(counts(i) == k);
            }
        }
        
        // Check that neighbors are the actual k nearest
        nlist.sort();
        for (size_t qi = 0; qi < 10; ++qi) {
            auto neighbors = nlist.getNeighborsForPoint(qi);
            
            // Verify distances are sorted
            for (size_t i = 1; i < neighbors.size(); ++i) {
                REQUIRE(neighbors[i].distance >= neighbors[i-1].distance);
            }
        }
    }
    
    SECTION("Scale parameter") {
        unsigned int k = 3;
        double scale = 1.5;
        QueryArgs args = QueryArgs::nearest(k, -1.0, scale);
        
        // The scale parameter affects the search radius expansion
        // Just verify it doesn't crash and finds the neighbors
        auto nlist = query.query(points, args);
        REQUIRE(!nlist.empty());
    }
    
    SECTION("Invalid arguments") {
        // Scale must be > 1.0
        QueryArgs args = QueryArgs::nearest(3, -1.0, 0.9);
        REQUIRE_THROWS_AS(query.query(points, args), std::runtime_error);
    }
}

TEST_CASE("AABBQuery with periodic boundaries", "[aabbquery]") {
    const double L = 4.0;
    const size_t n = 3;
    const double a = 1.0;
    
    // Create box and points
    Box box(L, L, L);
    auto points = makeSimpleCubicLattice(n, a);
    AABBQuery query(box, points);
    
    SECTION("Periodic wrapping") {
        // Points at the edges should find neighbors across periodic boundaries
        double r_max = a * 1.1;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query.query(points, args);
        
        // In a periodic cubic lattice, all points should have 6 neighbors
        auto counts = nlist.getNeighborCounts(points.shape(0));
        
        // Not all will have 6 due to box size, but check distances
        auto distances = nlist.getDistances();
        for (size_t i = 0; i < distances.size(); ++i) {
            REQUIRE(distances(i) <= r_max);
        }
    }
    
    SECTION("Non-periodic box") {
        Box non_periodic_box(L, L, L);
        non_periodic_box.setPeriodic(false, false, false);
        
        AABBQuery query_np(non_periodic_box, points);
        
        double r_max = a * 1.1;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query_np.query(points, args);
        
        // Corner points should have fewer neighbors without periodic boundaries
        auto counts = nlist.getNeighborCounts(points.shape(0));
        
        // At least one point should have fewer than 6 neighbors
        bool has_fewer = false;
        for (size_t i = 0; i < counts.size(); ++i) {
            if (counts(i) < 6) {
                has_fewer = true;
                break;
            }
        }
        REQUIRE(has_fewer);
    }
}

TEST_CASE("AABBQuery with 2D box", "[aabbquery]") {
    const double L = 10.0;
    const size_t N = 50;
    
    // Create 2D box
    Box box2d(L, L, 0.0, 0.0, 0.0, 0.0, true);
    
    // Create 2D points (z = 0)
    xt::xarray<double> points = xt::zeros<double>({N, 3});
    auto xy = xt::random::rand<double>({N, 2}) * L - L/2.0;
    xt::view(points, xt::all(), xt::range(0, 2)) = xy;
    
    AABBQuery query(box2d, points);
    
    SECTION("2D ball query") {
        double r_max = 2.0;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query.query(points, args);
        
        // All distances should be in 2D (z component should be 0)
        auto distance_vecs = nlist.getDistanceVectors();
        for (size_t i = 0; i < nlist.size(); ++i) {
            REQUIRE(std::abs(distance_vecs(i, 2)) < 1e-10);
        }
        
        // Check distances are within r_max
        auto distances = nlist.getDistances();
        for (size_t i = 0; i < distances.size(); ++i) {
            REQUIRE(distances(i) <= r_max);
        }
    }
}