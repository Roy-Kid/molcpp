#include <catch2/catch.hpp>
#include "molcpp/AABBQuery.hpp"
#include <xtensor/xtensor.hpp>
#include <unordered_set>

using namespace molcpp;

static std::size_t hash_pair(int a, int b) { return (static_cast<std::size_t>(a) << 32) ^ static_cast<std::size_t>(b); }

TEST_CASE("NeighborQuery: self query excludes ii and respects r_max") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}, {5.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 4);

    auto [i, j, d] = nq.query(pts, 1.01f, true);

    // All distances should be <= r_max
    for (std::size_t k = 0; k < d.size(); ++k) {
        REQUIRE(d(k) <= Approx(1.01f).epsilon(1e-5));
        REQUIRE(i(k) != j(k));
    }

    // Expected pairs: (0,1), (1,0,2), (2,1)
    std::unordered_set<std::size_t> pairs;
    for (std::size_t k = 0; k < i.size(); ++k) {
        pairs.insert(hash_pair(i(k), j(k)));
    }
    REQUIRE(pairs.count(hash_pair(0,1)) == 1);
    REQUIRE(pairs.count(hash_pair(1,0)) == 1);
    REQUIRE(pairs.count(hash_pair(1,2)) == 1);
    REQUIRE(pairs.count(hash_pair(2,1)) == 1);
}

TEST_CASE("NeighborQuery: self query includes ii when exclude_ii=false") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 4);

    auto [i, j, d] = nq.query(pts, 0.001f, false);

    // very small radius likely only finds self if allowed
    bool has_self = false;
    for (std::size_t k = 0; k < d.size(); ++k) {
        if (i(k) == j(k)) { has_self = true; }
        REQUIRE(d(k) <= Approx(0.001f).epsilon(1e-5));
    }
    REQUIRE(has_self);
}

TEST_CASE("NeighborQuery: PBC wrap-around across boundary") {
    xt::xtensor<float, 2> pts = {{0.1f,0.f,0.f}, {9.9f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 4);

    auto [i, j, d] = nq.query(pts, 0.25f, false);
    bool found = false;
    for (std::size_t k = 0; k < d.size(); ++k) {
        if (d(k) <= Approx(0.2f).epsilon(1e-3)) { found = true; break; }
    }
    REQUIRE(found);
}

TEST_CASE("NeighborQuery: query distinct set") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {3.f,0.f,0.f}};
    xt::xtensor<float, 2> q   = {{0.1f,0.f,0.f}, {2.9f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, false, false, false};
    AABBQuery nq(pts, bp, 2);

    auto [i, j, d] = nq.query(q, 0.2f, false);

    REQUIRE(i.size() == 2);
    REQUIRE(j.size() == 2);
    for (std::size_t k = 0; k < d.size(); ++k) REQUIRE(d(k) <= Approx(0.2f).epsilon(1e-5));
}

TEST_CASE("NeighborQuery: no neighbors for tiny radius") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {5.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, false, false, false};
    AABBQuery nq(pts, bp, 2);
    auto [i, j, d] = nq.query(pts, 0.01f, true);
    REQUIRE(i.size() == 0);
    REQUIRE(j.size() == 0);
    REQUIRE(d.size() == 0);
}

TEST_CASE("NeighborQuery: deterministic results across repeated calls") {
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}};
    BoxParams bp{10.f, 10.f, 10.f, true, true, true};
    AABBQuery nq(pts, bp, 2);

    auto r1 = nq.query(pts, 1.01f, true);
    auto r2 = nq.query(pts, 1.01f, true);

    auto& [i1, j1, d1] = r1;
    auto& [i2, j2, d2] = r2;

    REQUIRE(i1.size() == i2.size());
    REQUIRE(j1.size() == j2.size());
    REQUIRE(d1.size() == d2.size());
    for (std::size_t k = 0; k < d1.size(); ++k) {
        REQUIRE(i1(k) == i2(k));
        REQUIRE(j1(k) == j2(k));
        REQUIRE(d1(k) == Approx(d2(k)).epsilon(1e-6));
    }
}