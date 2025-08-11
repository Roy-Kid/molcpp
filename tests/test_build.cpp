#include "molcpp/AABBQuery.hpp"
#include <iostream>
#include <xtensor/xtensor.hpp>

// Replace this mock with your actual Box implementation in your environment
struct MockBox {
    float Lx_ = 10, Ly_ = 10, Lz_ = 10; bool px_=true, py_=true, pz_=true;
    float Lx() const { return Lx_; }
    float Ly() const { return Ly_; }
    float Lz() const { return Lz_; }
    bool periodic_x() const { return px_; }
    bool periodic_y() const { return py_; }
    bool periodic_z() const { return pz_; }
};

int main() {
    using namespace molcpp;
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}};
    MockBox box;
    AABBQuery nq(pts, box, 4);
    auto [ii, jj, dd] = nq.query(pts, 1.25f, false);
    std::cout << "pairs: " << ii.size() << "\n";
    return 0;
}