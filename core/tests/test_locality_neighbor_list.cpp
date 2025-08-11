#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xmath.hpp>

#include <molcpp/locality/NeighborList.hpp>
#include <molcpp/locality/AABBQuery.hpp>
#include <molcpp/box/Box.hpp>

using namespace molcpp;
using namespace molcpp::locality;
using namespace molcpp::box;

// Helper function to create random points
xt::xarray<double> makeRandomPoints(size_t N, double L) {
    return xt::random::rand<double>({N, 3}) * L - L/2.0;
}

TEST_CASE("NeighborList basic operations", "[neighborlist]") {
    SECTION("Construction and size") {
        NeighborList nlist;
        REQUIRE(nlist.empty());
        REQUIRE(nlist.size() == 0);
        
        nlist.addBond(0, 1, 1.5);
        REQUIRE(!nlist.empty());
        REQUIRE(nlist.size() == 1);
        
        nlist.addBond(0, 2, 2.0);
        REQUIRE(nlist.size() == 2);
        
        nlist.clear();
        REQUIRE(nlist.empty());
    }
    
    SECTION("Adding bonds") {
        NeighborList nlist;
        
        // Add bonds with different constructors
        nlist.addBond(0, 1, 1.5);
        nlist.addBond(1, 2, 2.0, 0.5, Vec3{1.0, 0.0, 0.0});
        
        NeighborBond bond(2, 3, 3.0);
        nlist.addBond(bond);
        
        REQUIRE(nlist.size() == 3);
        REQUIRE(nlist[0].query_point_idx == 0);
        REQUIRE(nlist[0].point_idx == 1);
        REQUIRE(nlist[0].distance == Approx(1.5));
        REQUIRE(nlist[0].weight == Approx(1.0));
        
        REQUIRE(nlist[1].weight == Approx(0.5));
        REQUIRE(nlist[2].distance == Approx(3.0));
    }
    
    SECTION("Sorting") {
        NeighborList nlist;
        nlist.addBond(2, 3, 3.0);
        nlist.addBond(0, 1, 1.0);
        nlist.addBond(1, 2, 2.0);
        nlist.addBond(0, 2, 1.5);
        
        REQUIRE(!nlist.isSorted());
        
        nlist.sort();
        REQUIRE(nlist.isSorted());
        
        // Check sorting order
        REQUIRE(nlist[0].query_point_idx == 0);
        REQUIRE(nlist[0].point_idx == 1);
        REQUIRE(nlist[1].query_point_idx == 0);
        REQUIRE(nlist[1].point_idx == 2);
        REQUIRE(nlist[2].query_point_idx == 1);
        REQUIRE(nlist[3].query_point_idx == 2);
    }
}

TEST_CASE("NeighborList filtering", "[neighborlist]") {
    NeighborList nlist;
    
    // Add bonds with various distances
    nlist.addBond(0, 1, 0.5);
    nlist.addBond(0, 2, 1.5);
    nlist.addBond(0, 3, 2.5);
    nlist.addBond(0, 4, 3.5);
    nlist.addBond(0, 5, 4.5);
    
    SECTION("Filter by r_max") {
        nlist.filterR(3.0);
        REQUIRE(nlist.size() == 3);
        
        // Check all remaining distances are <= 3.0
        for (size_t i = 0; i < nlist.size(); ++i) {
            REQUIRE(nlist[i].distance <= 3.0);
        }
    }
    
    SECTION("Filter by r_min and r_max") {
        NeighborList nlist2 = nlist;  // Make a copy
        nlist2.filterR(3.5, 1.0);
        REQUIRE(nlist2.size() == 3);
        
        // Check all remaining distances are in range [1.0, 3.5]
        for (size_t i = 0; i < nlist2.size(); ++i) {
            REQUIRE(nlist2[i].distance >= 1.0);
            REQUIRE(nlist2[i].distance <= 3.5);
        }
    }
    
    SECTION("Invalid filter arguments") {
        REQUIRE_THROWS_AS(nlist.filterR(-1.0), std::invalid_argument);
        REQUIRE_THROWS_AS(nlist.filterR(3.0, -1.0), std::invalid_argument);
        REQUIRE_THROWS_AS(nlist.filterR(1.0, 2.0), std::invalid_argument);
    }
}

TEST_CASE("NeighborList array access", "[neighborlist]") {
    NeighborList nlist;
    
    // Add some bonds
    for (unsigned int i = 0; i < 5; ++i) {
        for (unsigned int j = i + 1; j < 5; ++j) {
            double dist = static_cast<double>(j - i);
            nlist.addBond(i, j, dist);
        }
    }
    
    SECTION("Get arrays") {
        auto query_indices = nlist.getQueryPointIndices();
        auto point_indices = nlist.getPointIndices();
        auto distances = nlist.getDistances();
        auto weights = nlist.getWeights();
        
        REQUIRE(query_indices.size() == nlist.size());
        REQUIRE(point_indices.size() == nlist.size());
        REQUIRE(distances.size() == nlist.size());
        REQUIRE(weights.size() == nlist.size());
        
        // Check first bond
        REQUIRE(query_indices(0) == 0);
        REQUIRE(point_indices(0) == 1);
        REQUIRE(distances(0) == Approx(1.0));
        REQUIRE(weights(0) == Approx(1.0));
    }
    
    SECTION("Get neighbors for point") {
        auto neighbors = nlist.getNeighborsForPoint(0);
        REQUIRE(neighbors.size() == 4);  // Point 0 has neighbors 1, 2, 3, 4
        
        for (const auto& bond : neighbors) {
            REQUIRE(bond.query_point_idx == 0);
            REQUIRE(bond.point_idx > 0);
            REQUIRE(bond.point_idx <= 4);
        }
    }
    
    SECTION("Get neighbor counts") {
        auto counts = nlist.getNeighborCounts(5);
        REQUIRE(counts.size() == 5);
        REQUIRE(counts(0) == 4);  // Point 0 has 4 neighbors
        REQUIRE(counts(1) == 3);  // Point 1 has 3 neighbors
        REQUIRE(counts(2) == 2);  // Point 2 has 2 neighbors
        REQUIRE(counts(3) == 1);  // Point 3 has 1 neighbor
        REQUIRE(counts(4) == 0);  // Point 4 has 0 neighbors
    }
}

TEST_CASE("AABBQuery with NeighborList", "[aabbquery][neighborlist]") {
    const double L = 10.0;
    const size_t N = 40;
    
    // Create a box and random points
    Box box(L, L, L);
    auto points = makeRandomPoints(N, L);
    
    // Create AABBQuery
    AABBQuery query(box, points);
    
    SECTION("Ball query") {
        double r_max = 2.0;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto nlist = query.query(points, args);
        
        // Check that all distances are within r_max
        auto distances = nlist.getDistances();
        for (size_t i = 0; i < distances.size(); ++i) {
            REQUIRE(distances(i) <= r_max);
        }
        
        // Check no self-neighbors
        auto bonds = nlist.getBonds();
        for (const auto& bond : bonds) {
            REQUIRE(bond.query_point_idx != bond.point_idx);
        }
    }
    
    SECTION("K-nearest neighbors query") {
        unsigned int k = 6;
        QueryArgs args = QueryArgs::nearest(k, -1.0, 1.1, true);
        
        // Query only first 10 points to speed up test
        auto query_points = xt::view(points, xt::range(0, 10), xt::all());
        auto nlist = query.query(query_points, args);
        
        // Check that each query point has at most k neighbors
        auto counts = nlist.getNeighborCounts(10);
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(counts(i) <= k);
        }
        
        // Check neighbors are sorted by distance for each query point
        nlist.sort();
        for (size_t i = 0; i < 10; ++i) {
            auto neighbors = nlist.getNeighborsForPoint(i);
            for (size_t j = 1; j < neighbors.size(); ++j) {
                REQUIRE(neighbors[j].distance >= neighbors[j-1].distance);
            }
        }
    }
}