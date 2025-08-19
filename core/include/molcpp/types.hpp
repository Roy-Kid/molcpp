#ifndef MOLCPP_TYPES_HPP
#define MOLCPP_TYPES_HPP

#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>

template<typename T>
using Vec3 = xt::xtensor_fixed<T, xt::xshape<3>>;

template<typename T>
using Mat3 = xt::xtensor_fixed<T, xt::xshape<3, 3>>;

// Accepts shapes (3) for a single point, or (N, 3) for a batch
using XYZ = xt::xarray<float>;

#endif // MOLCPP_TYPES_HPP
