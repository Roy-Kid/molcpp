#include "core/include/molcpp/spatial/box.hpp"
#include <iostream>
#include <xtensor/containers/xarray.hpp>

using namespace molcpp;

int main() {
    Box box({2.0, 2.0, 2.0});
    
    // Test the specific failing case
    xt::xarray<double> points = {{-5.0, 0.0, 0.0}};
    auto wrapped = box.wrap(points);
    
    std::cout << "Input: [-5.0, 0.0, 0.0]" << std::endl;
    std::cout << "round(-5.0/2.0) = " << round(-5.0/2.0) << std::endl;
    std::cout << "-5.0 - 2*round(-5.0/2.0) = " << (-5.0 - 2.0*round(-5.0/2.0)) << std::endl;
    std::cout << "Wrapped: [" << wrapped(0,0) << ", " << wrapped(0,1) << ", " << wrapped(0,2) << "]" << std::endl;
    
    // Test large displacement
    xt::xarray<double> points2 = {{25.0, 0.0, 0.0}};
    auto wrapped2 = box.wrap(points2);
    std::cout << "\nInput: [25.0, 0.0, 0.0]" << std::endl;
    std::cout << "round(25.0/2.0) = " << round(25.0/2.0) << std::endl;
    std::cout << "25.0 - 2*round(25.0/2.0) = " << (25.0 - 2.0*round(25.0/2.0)) << std::endl;
    std::cout << "Wrapped: [" << wrapped2(0,0) << ", " << wrapped2(0,1) << ", " << wrapped2(0,2) << "]" << std::endl;
    
    return 0;
}
