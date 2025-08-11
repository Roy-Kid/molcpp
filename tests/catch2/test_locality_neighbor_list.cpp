#include <catch2/catch.hpp>
#include "molcpp/NeighborList.hpp"
#include "molcpp/AABBQuery.hpp"
#include <xtensor/xtensor.hpp>

using namespace molcpp;

TEST_CASE("NeighborList: sizes and accessors") {
    xt::xtensor<int, 1> i = {0, 1, 1};
    xt::xtensor<int, 1> j = {1, 0, 2};
    xt::xtensor<float, 1> d = {1.0f, 1.0f, 1.0f};
    NeighborList nl(std::move(i), std::move(j), std::move(d));
    REQUIRE(nl.size() == 3);
    REQUIRE(nl.i().size() == 3);
    REQUIRE(nl.j().size() == 3);
    REQUIRE(nl.distances().size() == 3);
}

TEST_CASE("NeighborList: integration with AABBQuery") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, false, false, false};
    AABBQuery nq(pts, bp, 2);
    NeighborList nl = nq.query_list(pts, 1.01f, true);
    REQUIRE(nl.size() >= 4); // at least (0,1),(1,0),(1,2),(2,1)
}