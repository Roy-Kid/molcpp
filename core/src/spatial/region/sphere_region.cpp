#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xmath.hpp>
#include <stdexcept>
#include <cmath>

namespace molcpp {

using XYZ = xt::xarray<float>;

SphereRegion::SphereRegion(const Vec3<float>& center, float radius)
    : _center(center), _radius(radius) {
    if (radius <= 0) {
        throw std::invalid_argument("Sphere radius must be positive");
    }
}

xt::xarray<bool> SphereRegion::isIn(const XYZ& points) const {
    // Handle single point vs batch
    if (points.dimension() == 1) {
        // Single point (3) -> (1, 3)
        XYZ points_batch = xt::reshape_view(points, {1, 3});
        return isIn(points_batch);
    }
    
    // Check shape: should be (n, 3)
    if (points.dimension() != 2 || points.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized distance calculation
    auto center_array = xt::xarray<float>{{_center[0], _center[1], _center[2]}};
    auto diff = points - center_array;
    auto dist_squared = xt::sum(diff * diff, 1);
    
    return dist_squared <= (_radius * _radius + TOLERANCE);
}

double SphereRegion::getVolume() const {
    constexpr double four_thirds_pi = 4.0 * M_PI / 3.0;
    return four_thirds_pi * _radius * _radius * _radius;
}

} // namespace molcpp
