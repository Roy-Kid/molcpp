#include "molcpp/AABBQuery.hpp"
#include <iostream>
#include <xtensor/xtensor.hpp>

#if __has_include("spatial/Box.hpp")
  #include "spatial/Box.hpp"
#elif __has_include("spatial/box.hpp")
  #include "spatial/box.hpp"
#else
  #error "Could not find user's Box header under src/spatial. Please ensure src/spatial/Box.hpp or src/spatial/box.hpp is available and add -I src to include dirs."
#endif

int main() {
    using namespace molcpp;
    xt::xtensor<float, 2> pts = {{0.f,0.f,0.f}, {1.f,0.f,0.f}, {2.f,0.f,0.f}};
    Box box(10.f, 10.f, 10.f, true, true, true);
    AABBQuery nq(pts, box, 4);
    auto [ii, jj, dd] = nq.query(pts, 1.25f, false);
    std::cout << "pairs: " << ii.size() << "\n";
    return 0;
}