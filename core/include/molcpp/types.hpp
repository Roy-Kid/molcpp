#ifndef MOLCPP_TYPES_HPP
#define MOLCPP_TYPES_HPP

#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>

// Explicit floating point type aliases - no default Real type
template<typename T>
using Vec3 = xt::xtensor_fixed<T, xt::xshape<3>>;

template<typename T>
using Mat3 = xt::xtensor_fixed<T, xt::xshape<3, 3>>;

// Explicit type aliases for common precisions
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Mat3f = Mat3<float>;
using Mat3d = Mat3<double>;

// Dynamic arrays with explicit types
template<typename T>
using CoordArray = xt::xarray<T>;  // For (N, 3) coordinate arrays

using CoordArrayf = CoordArray<float>;
using CoordArrayd = CoordArray<double>;

// Common type aliases for convenience
using Index = std::size_t;
using IntArray = xt::xarray<int>;

// Legacy aliases for backward compatibility
using XYZ = CoordArray<float>;

#endif // MOLCPP_TYPES_HPP
