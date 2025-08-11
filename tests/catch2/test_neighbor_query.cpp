#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "molcpp/AABBQuery.hpp"
#include <xtensor/xadapt.hpp>
#include <xtensor/xtensor.hpp>

using namespace molcpp;

TEST_CASE("AABBQuery basic radius query") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}, {5.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 4);

    xt::xtensor<float, 2> q = pts; // self-query
    auto [i, j, d] = nq.query(q, 1.01f, true);

    // Each point should find its immediate neighbors within 1.01 in 1D chain
    // Expected pairs: (0,1), (1,0,2), (2,1), (3, none)
    // Count should be 2 + 2 + 1 + 0 = 5 if exclude_ii true
    REQUIRE(d.size() >= 5);
}

TEST_CASE("AABBQuery PBC wrap around") {
    xt::xtensor<float, 2> pts = {{0.1f,0.f,0.f}, {9.9f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 4);

    auto [i, j, d] = nq.query(pts, 0.5f, false);
    // Points are 0.2 apart through PBC
    bool found = false;
    for (std::size_t k = 0; k < d.size(); ++k) {
        if (d(k) <= Approx(0.2f).epsilon(1e-3)) { found = true; break; }
    }
    REQUIRE(found);
}