#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xmath.hpp>
#include <stdexcept>

namespace molcpp {

using XYZ = xt::xarray<float>;

CubeRegion::CubeRegion(const Vec3<float>& lower_corner, float edge_length)
    : _lower_corner(lower_corner), _edge_lengths{edge_length, edge_length, edge_length} {
    if (edge_length <= 0) {
        throw std::invalid_argument("Cube edge length must be positive");
    }
}

CubeRegion::CubeRegion(const Vec3<float>& lower_corner, const Vec3<float>& edge_lengths)
    : _lower_corner(lower_corner), _edge_lengths(edge_lengths) {
    for (size_t i = 0; i < 3; ++i) {
        if (_edge_lengths[i] <= 0) {
            throw std::invalid_argument("Box edge lengths must be positive");
        }
    }
}

xt::xarray<bool> CubeRegion::isIn(const XYZ& points) const {
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

    // Vectorized bounds checking
    auto lower_bounds = xt::xarray<float>{{_lower_corner[0] - TOLERANCE, _lower_corner[1] - TOLERANCE, _lower_corner[2] - TOLERANCE}};
    auto upper_bounds = xt::xarray<float>{{_lower_corner[0] + _edge_lengths[0] + TOLERANCE, 
                                           _lower_corner[1] + _edge_lengths[1] + TOLERANCE, 
                                           _lower_corner[2] + _edge_lengths[2] + TOLERANCE}};
    
    // Check if all coordinates are within bounds for each particle
    auto above_lower = points >= lower_bounds;
    auto below_upper = points <= upper_bounds;
    auto within_bounds = above_lower && below_upper;
    
    // Sum along axis 1 to count how many coordinates are within bounds per particle
    auto count_within = xt::sum(within_bounds, {1});
    return xt::equal(count_within, 3);
}

double CubeRegion::getVolume() const {
    return _edge_lengths[0] * _edge_lengths[1] * _edge_lengths[2];
}

} // namespace molcpp
