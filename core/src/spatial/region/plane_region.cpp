#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xmath.hpp>
#include <stdexcept>
#include <cmath>
#include <limits>

namespace molcpp {

using XYZ = xt::xarray<float>;

PlaneRegion::PlaneRegion(const Vec3<float>& point, const Vec3<float>& normal, float thickness)
    : _point(point), _normal(normal), _thickness(thickness) {
    
    if (thickness <= 0) {
        throw std::invalid_argument("Plane thickness must be positive");
    }
    
    // Normalize the normal vector
    float norm = std::sqrt(_normal[0]*_normal[0] + _normal[1]*_normal[1] + _normal[2]*_normal[2]);
    if (norm < TOLERANCE) {
        throw std::invalid_argument("Normal vector cannot be zero");
    }
    
    _normal[0] /= norm;
    _normal[1] /= norm;
    _normal[2] /= norm;
}

xt::xarray<bool> PlaneRegion::isIn(const XYZ& points) const {
    // Handle single point vs batch
    if (points.dimension() == 1) {
        // Single point (3) -> (1, 3)
        XYZ points_batch = xt::reshape_view(points, {1, 3});
        return isIn(points_batch);
    }
    
    if (points.dimension() != 2 || points.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized plane distance calculation
    auto point_array = xt::xarray<float>{{_point[0], _point[1], _point[2]}};
    auto normal_array = xt::xarray<float>{{_normal[0], _normal[1], _normal[2]}};
    
    // Vector from plane point to each coordinate
    auto diff_vectors = points - point_array;
    
    // Calculate distances to plane using element-wise multiplication and sum
    auto distances = xt::sum(diff_vectors * normal_array, 1);
    
    // Check if distance is within thickness
    return xt::abs(distances) <= (_thickness + TOLERANCE);
}

double PlaneRegion::getVolume() const {
    return std::numeric_limits<double>::infinity();
}

} // namespace molcpp
