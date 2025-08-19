#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xmath.hpp>
#include <stdexcept>
#include <cmath>

namespace molcpp {

using XYZ = xt::xarray<float>;

CylinderRegion::CylinderRegion(const Vec3<float>& end1, const Vec3<float>& end2, float radius)
    : _end1(end1), _end2(end2), _radius(radius) {
    
    if (radius <= 0) {
        throw std::invalid_argument("Cylinder radius must be positive");
    }
    
    // Calculate axis and height
    _axis[0] = end2[0] - end1[0];
    _axis[1] = end2[1] - end1[1];
    _axis[2] = end2[2] - end1[2];
    
    _height = std::sqrt(_axis[0]*_axis[0] + _axis[1]*_axis[1] + _axis[2]*_axis[2]);
    
    if (_height < TOLERANCE) {
        throw std::invalid_argument("Cylinder centers must be distinct");
    }
    
    // Normalize axis
    _axis[0] /= _height;
    _axis[1] /= _height;
    _axis[2] /= _height;
}

xt::xarray<bool> CylinderRegion::isIn(const XYZ& points) const {
    // Handle single point vs batch
    if (points.dimension() == 1) {
        // Single point (3) -> (1, 3)
        XYZ points_batch = xt::reshape_view(points, {1, 3});
        return isIn(points_batch);
    }
    
    if (points.dimension() != 2 || points.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized cylinder containment check
    auto end1_array = xt::xarray<float>{{_end1[0], _end1[1], _end1[2]}};
    auto axis_array = xt::xarray<float>{{_axis[0], _axis[1], _axis[2]}};
    
    // Vector from end1 to points
    auto p_vectors = points - end1_array;
    
    // Project onto cylinder axis using element-wise multiplication and sum
    auto proj_lengths = xt::sum(p_vectors * axis_array, 1);
    
    // Check if projections are within cylinder height
    auto height_check = (proj_lengths >= -TOLERANCE) && (proj_lengths <= _height + TOLERANCE);
    
    // Calculate perpendicular distances
    auto proj_vectors = xt::expand_dims(proj_lengths, 1) * axis_array;
    auto perp_vectors = p_vectors - proj_vectors;
    auto perp_dist_sq = xt::sum(perp_vectors * perp_vectors, 1);
    
    // Check if perpendicular distances are within radius
    auto radius_check = perp_dist_sq <= (_radius * _radius + TOLERANCE);
    
    return height_check && radius_check;
}

double CylinderRegion::getVolume() const {
    return M_PI * _radius * _radius * _height;
}

} // namespace molcpp
