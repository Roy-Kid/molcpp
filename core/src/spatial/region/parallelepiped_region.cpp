#include "molcpp/spatial/region.hpp"

#include <xtensor/containers/xarray.hpp>
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/views/xview.hpp>

namespace molcpp {

using XYZ = xt::xarray<float>;


ParallelepipedRegion::ParallelepipedRegion(const Mat3<float>& matrix, const Vec3<float>& origin)
    : _matrix(matrix), _origin(origin) {}

xt::xarray<bool> ParallelepipedRegion::isIn(const XYZ& points) const {
    // Convert to fractional coordinates and test membership
    // frac = H^{-1} @ (points - origin)
    
    // Calculate inverse matrix
    XYZ inv_matrix = xt::linalg::inv(_matrix);
    
    // Handle single point vs batch
    if (points.dimension() == 1) {
        // Single point (3) -> (1, 3)
        XYZ points_batch = xt::reshape_view(points, {1, 3});
        XYZ frac = xt::linalg::dot(points_batch - _origin, inv_matrix);
        
        // Test if all fractional coordinates are in [0, 1]
        auto in_range = (frac >= 0.0f) && (frac <= 1.0f);
        return xt::all(in_range);
    } else {
        // Batch (N, 3)
        XYZ frac = xt::linalg::dot(points - _origin, inv_matrix);
        
        // Test if all fractional coordinates are in [0, 1]
        auto in_range = (frac >= 0.0f) && (frac <= 1.0f);
        
        // For batch, we need to check that all 3 coordinates are in range for each point
        // Create a result array of shape (N)
        auto result = xt::xarray<bool>::from_shape({points.shape()[0]});
        
        // For each point, check if all coordinates are in range
        for (size_t i = 0; i < points.shape()[0]; ++i) {
            // Check if all 3 coordinates for this point are in range
            bool all_in_range = true;
            for (size_t j = 0; j < 3; ++j) {
                if (!in_range(i, j)) {
                    all_in_range = false;
                    break;
                }
            }
            result(i) = all_in_range;
        }
        
        return result;
    }
}

double ParallelepipedRegion::getVolume() const {
    // Volume = |det(H)|
    return std::abs(xt::linalg::det(_matrix));
}

} // namespace molcpp
