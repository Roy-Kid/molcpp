#ifndef MOLCPP_TYPES_HPP
#define MOLCPP_TYPES_HPP

#include <xtensor/containers/xfixed.hpp>

typedef xt::xtensor_fixed<double, xt::xshape<3>> Vec3;
typedef xt::xtensor_fixed<double, xt::xshape<3, 3>> Mat3;

#endif // MOLCPP_TYPES_HPP
