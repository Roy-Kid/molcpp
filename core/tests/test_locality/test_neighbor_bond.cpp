#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/locality/NeighborBond.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::locality;
using namespace molcpp;

TEST_CASE("NeighborBond construction and basic functionality", "[locality][neighbor_bond]")
{
    SECTION("Default constructor")
    {
        NeighborBond bond;
        CHECK(bond.getQueryPointIdx() == 0);
        CHECK(bond.getPointIdx() == 0);
        CHECK(bond.getDistance() == 0.0f);
        CHECK(bond.getWeight() == 0.0f);
    }

    SECTION("Constructor with distance")
    {
        Vec3<float> vec{1.0f, 2.0f, 3.0f};
        NeighborBond bond(1, 2, 3.741657f, 0.5f, vec);
        
        CHECK(bond.getQueryPointIdx() == 1);
        CHECK(bond.getPointIdx() == 2);
        CHECK(bond.getDistance() == 3.741657f);
        CHECK(bond.getWeight() == 0.5f);
        CHECK(bond.getVector() == vec);
    }

    SECTION("Constructor without distance (auto-computed)")
    {
        Vec3<float> vec{3.0f, 4.0f, 0.0f}; // length = 5.0
        NeighborBond bond(0, 1, 1.0f, vec);
        
        CHECK(bond.getQueryPointIdx() == 0);
        CHECK(bond.getPointIdx() == 1);
        CHECK(bond.getDistance() == Catch::Approx(5.0f));
        CHECK(bond.getWeight() == 1.0f);
        CHECK(bond.getVector() == vec);
    }
}

TEST_CASE("NeighborBond setters", "[locality][neighbor_bond]")
{
    NeighborBond bond;
    
    SECTION("Set query point index")
    {
        bond.setQueryPointIdx(5);
        CHECK(bond.getQueryPointIdx() == 5);
    }
    
    SECTION("Set point index")
    {
        bond.setPointIdx(10);
        CHECK(bond.getPointIdx() == 10);
    }
    
    SECTION("Set weight")
    {
        bond.setWeight(2.5f);
        CHECK(bond.getWeight() == 2.5f);
    }
    
    SECTION("Set vector")
    {
        Vec3<float> vec{1.0f, 0.0f, 0.0f}; // length = 1.0
        bond.setVector(vec);
        CHECK(bond.getVector() == vec);
        CHECK(bond.getDistance() == Catch::Approx(1.0f));
    }
}

TEST_CASE("NeighborBond comparison operators", "[locality][neighbor_bond]")
{
    NeighborBond bond1(0, 1, 2.0f, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});
    NeighborBond bond2(0, 2, 3.0f, 1.0f, Vec3<float>{0.0f, 1.0f, 0.0f});
    NeighborBond bond3(0, 1, 2.0f, 1.0f, Vec3<float>{1.0f, 0.0f, 0.0f});
    
    SECTION("Equality operator")
    {
        CHECK(bond1 == bond3);
        CHECK_FALSE(bond1 == bond2);
    }
    
    SECTION("Inequality operator")
    {
        CHECK(bond1 != bond2);
        CHECK_FALSE(bond1 != bond3);
    }
    
    SECTION("Less than operator (by distance)")
    {
        CHECK(bond1 < bond2); // 2.0 < 3.0
        CHECK_FALSE(bond2 < bond1);
    }
}

TEST_CASE("NeighborBond comparison methods", "[locality][neighbor_bond]")
{
    NeighborBond bond1(0, 1, 2.0f, 0.5f, Vec3<float>{1.0f, 0.0f, 0.0f});
    NeighborBond bond2(0, 2, 2.0f, 0.5f, Vec3<float>{0.0f, 1.0f, 0.0f});
    NeighborBond bond3(1, 1, 2.0f, 0.5f, Vec3<float>{1.0f, 0.0f, 0.0f});
    
    SECTION("lessAsTuple")
    {
        CHECK(bond1.lessAsTuple(bond3)); // query_point_idx: 0 < 1
        CHECK_FALSE(bond3.lessAsTuple(bond1));
        
        NeighborBond bond4(0, 1, 2.0f, 0.5f, Vec3<float>{1.0f, 0.0f, 0.0f});
        CHECK_FALSE(bond1.lessAsTuple(bond4)); // identical
    }
    
    SECTION("lessAsDistance")
    {
        CHECK(bond1.lessAsDistance(bond3)); // query_point_idx: 0 < 1
        CHECK_FALSE(bond3.lessAsDistance(bond1));
    }
    
    SECTION("lessIdRefWeight")
    {
        CHECK(bond1.lessIdRefWeight(bond3)); // query_point_idx: 0 < 1
        CHECK_FALSE(bond3.lessIdRefWeight(bond1));
    }
}
