#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/locality/NeighborList.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/spatial/boundary.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::locality;
using namespace molcpp;

TEST_CASE("NeighborList construction", "[locality][neighbor_list]")
{
    SECTION("Default constructor")
    {
        NeighborList nlist;
        CHECK(nlist.getNumBonds() == 0);
        CHECK(nlist.getNumQueryPoints() == 0);
        CHECK(nlist.getNumPoints() == 0);
    }

    SECTION("Constructor with num_bonds")
    {
        NeighborList nlist(10);
        CHECK(nlist.getNumBonds() == 10);
        CHECK(nlist.getNumQueryPoints() == 0);
        CHECK(nlist.getNumPoints() == 0);
    }

    SECTION("Copy constructor")
    {
        NeighborList nlist1(5);
        NeighborList nlist2(nlist1);
        CHECK(nlist2.getNumBonds() == 5);
        CHECK(nlist2.getNumQueryPoints() == 0);
        CHECK(nlist2.getNumPoints() == 0);
    }
}

TEST_CASE("NeighborList from NeighborBonds", "[locality][neighbor_list]")
{
    std::vector<NeighborBond> bonds;
    bonds.emplace_back(0, 1, 2.0f, Vec3<float>{1.0f, 0.0f, 0.0f});
    bonds.emplace_back(0, 2, 3.0f, Vec3<float>{0.0f, 1.0f, 0.0f});
    bonds.emplace_back(1, 0, 2.5f, Vec3<float>{0.0f, 0.0f, 1.0f});

    NeighborList nlist(bonds);
    
    CHECK(nlist.getNumBonds() == 3);
    CHECK(nlist.getNumQueryPoints() == 2); // max query_point_idx + 1
    CHECK(nlist.getNumPoints() == 3);      // max point_idx + 1
}

TEST_CASE("NeighborList from points and box", "[locality][neighbor_list]")
{
    // Create a simple cubic box
    auto box = Box::cube(10.0f, Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<bool>{true, true, true});
    
    // Create some test points
    Vec3<float> points[] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    };
    
    Vec3<float> query_points[] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}
    };

    SECTION("Without excluding self-interactions")
    {
        NeighborList nlist(points, query_points, box, false, 3, 2);
        CHECK(nlist.getNumBonds() == 6); // 3 * 2
        CHECK(nlist.getNumQueryPoints() == 2);
        CHECK(nlist.getNumPoints() == 3);
    }

    SECTION("Excluding self-interactions")
    {
        NeighborList nlist(points, query_points, box, true, 3, 2);
        CHECK(nlist.getNumBonds() == 4); // 3 * 2 - 2 (self-interactions)
        CHECK(nlist.getNumQueryPoints() == 2);
        CHECK(nlist.getNumPoints() == 3);
    }
}

TEST_CASE("NeighborList filtering", "[locality][neighbor_list]")
{
    std::vector<NeighborBond> bonds;
    // Use vectors that actually produce the expected distances
    bonds.emplace_back(0, 1, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});  // distance = 1.0
    bonds.emplace_back(0, 2, 1.0f, Vec3<float>{2.0f, 0.0f, 0.0f});  // distance = 2.0
    bonds.emplace_back(0, 3, 1.0f, Vec3<float>{3.0f, 0.0f, 0.0f});  // distance = 3.0
    bonds.emplace_back(1, 0, 1.0f, Vec3<float>{1.5f, 0.0f, 0.0f});  // distance = 1.5

    NeighborList nlist(bonds);
    
    SECTION("Filter by distance range")
    {
        unsigned int removed = nlist.filterR(2.5f, 1.0f);
        CHECK(removed == 1); // Only the bond with distance 3.0 should be removed
        CHECK(nlist.getNumBonds() == 3);
    }

    SECTION("Filter by boolean array")
    {
        std::vector<bool> filter{true, false, true, false};
        unsigned int removed = nlist.filter(filter.begin());
        CHECK(removed == 2); // 2 bonds removed
        CHECK(nlist.getNumBonds() == 2);
    }
}

TEST_CASE("NeighborList sorting", "[locality][neighbor_list]")
{
    std::vector<NeighborBond> bonds;
    // Use vectors that actually produce the expected distances
    bonds.emplace_back(0, 1, 1.0f, Vec3<float>{3.0f, 0.0f, 0.0f});  // distance = 3.0
    bonds.emplace_back(0, 2, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});  // distance = 1.0
    bonds.emplace_back(0, 3, 1.0f, Vec3<float>{2.0f, 0.0f, 0.0f});  // distance = 2.0

    NeighborList nlist(bonds);
    
    SECTION("Sort by distance")
    {
        nlist.sort(true);
        auto distances = nlist.getDistances();
        CHECK((*distances)[0] == Catch::Approx(1.0f));
        CHECK((*distances)[1] == Catch::Approx(2.0f));
        CHECK((*distances)[2] == Catch::Approx(3.0f));
    }

    SECTION("Sort by bond order")
    {
        nlist.sort(false);
        auto neighbors = nlist.getNeighbors();
        // Should maintain the original order since query_point_idx is the same
        CHECK((*neighbors)(0, 0) == 0);
        CHECK((*neighbors)(1, 0) == 0);
        CHECK((*neighbors)(2, 0) == 0);
    }
}

TEST_CASE("NeighborList utility methods", "[locality][neighbor_list]")
{
    std::vector<NeighborBond> bonds;
    bonds.emplace_back(0, 1, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});
    bonds.emplace_back(0, 2, 2.0f, Vec3<float>{0.0f, 1.0f, 0.0f});
    bonds.emplace_back(1, 0, 1.5f, Vec3<float>{1.0f, 1.0f, 0.0f});

    NeighborList nlist(bonds);
    
    SECTION("findFirstIndex")
    {
        CHECK(nlist.findFirstIndex(0) == 0);
        CHECK(nlist.findFirstIndex(1) == 2);
    }

    SECTION("resize")
    {
        nlist.resize(2);
        CHECK(nlist.getNumBonds() == 2);
    }

    SECTION("setNumBonds")
    {
        nlist.setNumBonds(5, 3, 4);
        CHECK(nlist.getNumBonds() == 5);
        CHECK(nlist.getNumQueryPoints() == 3);
        CHECK(nlist.getNumPoints() == 4);
    }

    SECTION("validate")
    {
        // Should not throw
        CHECK_NOTHROW(nlist.validate(2, 3));
        
        // Should throw for mismatched sizes
        CHECK_THROWS(nlist.validate(3, 3));
        CHECK_THROWS(nlist.validate(2, 4));
    }
}

TEST_CASE("NeighborList conversion to bond vector", "[locality][neighbor_list]")
{
    std::vector<NeighborBond> original_bonds;
    original_bonds.emplace_back(0, 1, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});
    original_bonds.emplace_back(0, 2, 2.0f, Vec3<float>{0.0f, 1.0f, 0.0f});

    NeighborList nlist(original_bonds);
    auto converted_bonds = nlist.toBondVector();
    
    CHECK(converted_bonds.size() == 2);
    CHECK(converted_bonds[0].getQueryPointIdx() == 0);
    CHECK(converted_bonds[0].getPointIdx() == 1);
    CHECK(converted_bonds[1].getQueryPointIdx() == 0);
    CHECK(converted_bonds[1].getPointIdx() == 2);
}
