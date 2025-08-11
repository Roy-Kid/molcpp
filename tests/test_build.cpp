#include "molcpp/box.hpp"
#include "molcpp/neighbor_aabb.hpp"
#include <iostream>
#include <xtensor/xtensor.hpp>

int main() {
    using namespace molcpp;
    Box box(10.0f, 10.0f, 10.0f, true, true, true);
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f},{1.f,0.f,0.f},{2.f,0.f,0.f},{3.f,0.f,0.f},{4.f,0.f,0.f},{5.f,0.f,0.f}};
    AABBNeighborQuery nq(box, pts, 4);
    xt::xtensor<float, 2> q = {{0.1f,0.f,0.f},{4.9f,0.f,0.f}};
    auto [ii, jj, dd] = nq.query(q, 1.25f, false);
    std::cout << "pairs: " << ii.size() << "\n";
    return 0;
}