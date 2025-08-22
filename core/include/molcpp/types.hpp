#ifndef MOLCPP_TYPES_HPP
#define MOLCPP_TYPES_HPP

#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>

// Default floating point type (can be overridden by CMake)
#ifdef MOLCPP_USE_DOUBLE
    using Real = double;
#else
    using Real = float;
#endif

// Template-based types for flexibility
template<typename T = Real>
using Vec3 = xt::xtensor_fixed<T, xt::xshape<3>>;

template<typename T = Real>
using Mat3 = xt::xtensor_fixed<T, xt::xshape<3, 3>>;

// Dynamic arrays with default type
template<typename T = Real>
using RealArray = xt::xarray<T>;

template<typename T = Real>
using CoordArray = xt::xarray<T>;  // For (N, 3) coordinate arrays

// Common type aliases for convenience
using Index = std::size_t;
using IntArray = xt::xarray<int>;

// Legacy aliases for backward compatibility
using XYZ = CoordArray<float>;

#endif // MOLCPP_TYPES_HPP
