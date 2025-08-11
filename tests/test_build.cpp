#include "molcpp/types.hpp"
#include "molcpp/box.hpp"
#include "molcpp/neighbor_aabb.hpp"
#include <iostream>
#include <vector>

int main() {
    using namespace molcpp;
    Box box(10.0f, 10.0f, 10.0f, true, true, true);
    std::vector<Vec3f> pts;
    for (int i = 0; i < 10; ++i) pts.emplace_back(static_cast<float>(i), 0.0f, 0.0f);
    AABBNeighborQuery nq(box, pts, 4);
    std::vector<Vec3f> q{{0.1f, 0.0f, 0.0f}, {9.9f, 0.0f, 0.0f}};
    auto [ii, jj, dd] = nq.query(q, 1.25f, false);
    std::cout << "pairs: " << ii.size() << "\n";
    return 0;
}