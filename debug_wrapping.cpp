#include <iostream>
#include "molcpp/spatial/box.hpp"
#include <xtensor/containers/xarray.hpp>

using namespace molcpp;

int main() {
    Box box({2.0, 2.0, 2.0});
    
    // Test wrapping behavior
    xt::xarray<double> points = {{3.0, -0.5, 0.5}};
    auto wrapped = box.wrap(points);
    
    std::cout << "Input: [3.0, -0.5, 0.5]" << std::endl;
    std::cout << "Wrapped: [" << wrapped(0, 0) << ", " << wrapped(0, 1) << ", " << wrapped(0, 2) << "]" << std::endl;
    
    // Test another point
    xt::xarray<double> points2 = {{0.0, 0.0, 0.0}};
    auto wrapped2 = box.wrap(points2);
    std::cout << "Input: [0.0, 0.0, 0.0]" << std::endl;
    std::cout << "Wrapped: [" << wrapped2(0, 0) << ", " << wrapped2(0, 1) << ", " << wrapped2(0, 2) << "]" << std::endl;
    
    // Test another point
    xt::xarray<double> points3 = {{2.0, 2.0, 2.0}};
    auto wrapped3 = box.wrap(points3);
    std::cout << "Input: [2.0, 2.0, 2.0]" << std::endl;
    std::cout << "Wrapped: [" << wrapped3(0, 0) << ", " << wrapped3(0, 1) << ", " << wrapped3(0, 2) << "]" << std::endl;
    
    return 0;
}
