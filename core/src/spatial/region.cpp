#include "molcpp/spatial/region.hpp"
#include <stdexcept>
#include <limits>
#include <cmath>
#include <xtensor/core/xmath.hpp>
#include <xtensor/views/xview.hpp>
#include <xtensor-blas/xlinalg.hpp>

namespace molcpp {

// InsideCube implementation
InsideCube::InsideCube(const Vec3& lo, double L) : lo_(lo), lengths_{L, L, L} {
    if (L <= 0) {
        throw std::invalid_argument("Cube edge length must be positive");
    }
}

InsideCube::InsideCube(const Vec3& lo, const Vec3& lengths) : lo_(lo), lengths_(lengths) {
    for (size_t i = 0; i < 3; ++i) {
        if (lengths_[i] <= 0) {
            throw std::invalid_argument("Box edge lengths must be positive");
        }
    }
}

xt::xarray<bool> InsideCube::isin(const xt::xarray<double>& coords) const {
    // Check shape: should be (n, 3)
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized bounds checking
    auto lower_bounds = xt::xarray<double>{{lo_[0] - TOLERANCE, lo_[1] - TOLERANCE, lo_[2] - TOLERANCE}};
    auto upper_bounds = xt::xarray<double>{{lo_[0] + lengths_[0] + TOLERANCE, 
                                           lo_[1] + lengths_[1] + TOLERANCE, 
                                           lo_[2] + lengths_[2] + TOLERANCE}};
    
    // Check if all coordinates are within bounds for each particle
    auto above_lower = coords >= lower_bounds;
    auto below_upper = coords <= upper_bounds;
    auto within_bounds = above_lower && below_upper;
    
    // Sum along axis 1 to count how many coordinates are within bounds per particle
    auto count_within = xt::sum(within_bounds, {1});
    return xt::equal(count_within, 3);
}

std::array<double, 6> InsideCube::boundary() const {
    return {lo_[0], lo_[0] + lengths_[0], 
            lo_[1], lo_[1] + lengths_[1], 
            lo_[2], lo_[2] + lengths_[2]};
}

double InsideCube::volume() const {
    return lengths_[0] * lengths_[1] * lengths_[2];
}

// InsideSphere implementation
InsideSphere::InsideSphere(const Vec3& center, double R) : center_(center), R_(R) {
    if (R <= 0) {
        throw std::invalid_argument("Sphere radius must be positive");
    }
}

xt::xarray<bool> InsideSphere::isin(const xt::xarray<double>& coords) const {
    // Check shape: should be (n, 3)
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized distance calculation
    auto center_array = xt::xarray<double>{{center_[0], center_[1], center_[2]}};
    auto diff = coords - center_array;
    auto dist_squared = xt::sum(diff * diff, 1);
    
    return dist_squared <= (R_ * R_ + TOLERANCE);
}

std::array<double, 6> InsideSphere::boundary() const {
    return {
        center_[0] - R_, center_[0] + R_,
        center_[1] - R_, center_[1] + R_,
        center_[2] - R_, center_[2] + R_
    };
}

double InsideSphere::volume() const {
    constexpr double four_thirds_pi = 4.0 * M_PI / 3.0;
    return four_thirds_pi * R_ * R_ * R_;
}

// InsideCylinder implementation
InsideCylinder::InsideCylinder(const Vec3& center1, const Vec3& center2, double radius)
    : center1_(center1), center2_(center2), radius_(radius) {
    
    if (radius <= 0) {
        throw std::invalid_argument("Cylinder radius must be positive");
    }
    
    // Calculate axis and height
    axis_[0] = center2_[0] - center1_[0];
    axis_[1] = center2_[1] - center1_[1];
    axis_[2] = center2_[2] - center1_[2];
    
    height_ = std::sqrt(axis_[0]*axis_[0] + axis_[1]*axis_[1] + axis_[2]*axis_[2]);
    
    if (height_ < TOLERANCE) {
        throw std::invalid_argument("Cylinder centers must be distinct");
    }
    
    // Normalize axis
    axis_[0] /= height_;
    axis_[1] /= height_;
    axis_[2] /= height_;
}

xt::xarray<bool> InsideCylinder::isin(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized cylinder containment check
    auto center1_array = xt::xarray<double>{{center1_[0], center1_[1], center1_[2]}};
    auto axis_array = xt::xarray<double>{{axis_[0], axis_[1], axis_[2]}};
    
    // Vector from center1 to points
    auto p_vectors = coords - center1_array;
    
    // Project onto cylinder axis using element-wise multiplication and sum
    auto proj_lengths = xt::sum(p_vectors * axis_array, 1);
    
    // Check if projections are within cylinder height
    auto height_check = (proj_lengths >= -TOLERANCE) && (proj_lengths <= height_ + TOLERANCE);
    
    // Calculate perpendicular distances
    auto proj_vectors = xt::expand_dims(proj_lengths, 1) * axis_array;
    auto perp_vectors = p_vectors - proj_vectors;
    auto perp_dist_sq = xt::sum(perp_vectors * perp_vectors, 1);
    
    // Check if perpendicular distances are within radius
    auto radius_check = perp_dist_sq <= (radius_ * radius_ + TOLERANCE);
    
    return height_check && radius_check;
}

std::array<double, 6> InsideCylinder::boundary() const {
    double xmin = std::min(center1_[0], center2_[0]) - radius_;
    double xmax = std::max(center1_[0], center2_[0]) + radius_;
    double ymin = std::min(center1_[1], center2_[1]) - radius_;
    double ymax = std::max(center1_[1], center2_[1]) + radius_;
    double zmin = std::min(center1_[2], center2_[2]) - radius_;
    double zmax = std::max(center1_[2], center2_[2]) + radius_;
    
    return {xmin, xmax, ymin, ymax, zmin, zmax};
}

double InsideCylinder::volume() const {
    return M_PI * radius_ * radius_ * height_;
}

double InsideCylinder::get_height() const {
    return height_;
}

// NearPlane implementation
NearPlane::NearPlane(const Vec3& point, const Vec3& normal, double thickness)
    : point_(point), normal_(normal), thickness_(thickness) {
    
    if (thickness <= 0) {
        throw std::invalid_argument("Plane thickness must be positive");
    }
    
    // Normalize the normal vector
    double norm = std::sqrt(normal_[0]*normal_[0] + normal_[1]*normal_[1] + normal_[2]*normal_[2]);
    if (norm < TOLERANCE) {
        throw std::invalid_argument("Normal vector cannot be zero");
    }
    
    normal_[0] /= norm;
    normal_[1] /= norm;
    normal_[2] /= norm;
}

xt::xarray<bool> NearPlane::isin(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized plane distance calculation
    auto point_array = xt::xarray<double>{{point_[0], point_[1], point_[2]}};
    auto normal_array = xt::xarray<double>{{normal_[0], normal_[1], normal_[2]}};
    
    // Vector from plane point to each coordinate
    auto diff_vectors = coords - point_array;
    
    // Calculate distances to plane using element-wise multiplication and sum
    auto distances = xt::sum(diff_vectors * normal_array, 1);
    
    // Check if distance is within thickness
    return xt::abs(distances) <= (thickness_ + TOLERANCE);
}

std::array<double, 6> NearPlane::boundary() const {
    return {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
}

double NearPlane::volume() const {
    return std::numeric_limits<double>::infinity();
}

// AndRegion implementation
AndRegion::AndRegion(std::vector<std::shared_ptr<Region>> regions)
    : regions_(std::move(regions)) {
    if (regions_.empty()) {
        throw std::invalid_argument("AndRegion requires at least one region");
    }
}

AndRegion::AndRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2) {
    regions_.push_back(std::move(region1));
    regions_.push_back(std::move(region2));
}

xt::xarray<bool> AndRegion::isin(const xt::xarray<double>& coords) const {
    if (regions_.empty()) {
        return xt::xarray<bool>();
    }
    
    auto result = regions_[0]->isin(coords);
    for (size_t i = 1; i < regions_.size(); ++i) {
        result = result && regions_[i]->isin(coords);
    }
    return result;
}

std::array<double, 6> AndRegion::boundary() const {
    auto result = regions_[0]->boundary();
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        auto bounds = regions_[i]->boundary();
        // Intersection of bounds
        result[0] = std::max(result[0], bounds[0]); // xmin
        result[1] = std::min(result[1], bounds[1]); // xmax
        result[2] = std::max(result[2], bounds[2]); // ymin
        result[3] = std::min(result[3], bounds[3]); // ymax
        result[4] = std::max(result[4], bounds[4]); // zmin
        result[5] = std::min(result[5], bounds[5]); // zmax
    }
    
    return result;
}

double AndRegion::volume() const {
    // Volume calculation for intersection is complex, return NaN
    return std::numeric_limits<double>::quiet_NaN();
}

// OrRegion implementation
OrRegion::OrRegion(std::vector<std::shared_ptr<Region>> regions)
    : regions_(std::move(regions)) {
    if (regions_.empty()) {
        throw std::invalid_argument("OrRegion requires at least one region");
    }
}

OrRegion::OrRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2) {
    regions_.push_back(std::move(region1));
    regions_.push_back(std::move(region2));
}

xt::xarray<bool> OrRegion::isin(const xt::xarray<double>& coords) const {
    if (regions_.empty()) {
        return xt::xarray<bool>();
    }
    
    auto result = regions_[0]->isin(coords);
    for (size_t i = 1; i < regions_.size(); ++i) {
        result = result || regions_[i]->isin(coords);
    }
    return result;
}

std::array<double, 6> OrRegion::boundary() const {
    auto result = regions_[0]->boundary();
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        auto bounds = regions_[i]->boundary();
        // Union of bounds
        result[0] = std::min(result[0], bounds[0]); // xmin
        result[1] = std::max(result[1], bounds[1]); // xmax
        result[2] = std::min(result[2], bounds[2]); // ymin
        result[3] = std::max(result[3], bounds[3]); // ymax
        result[4] = std::min(result[4], bounds[4]); // zmin
        result[5] = std::max(result[5], bounds[5]); // zmax
    }
    
    return result;
}

double OrRegion::volume() const {
    // Volume calculation for union is complex, return NaN
    return std::numeric_limits<double>::quiet_NaN();
}

// NotRegion implementation
NotRegion::NotRegion(std::shared_ptr<Region> region) : region_(std::move(region)) {
    if (!region_) {
        throw std::invalid_argument("NotRegion requires a valid region");
    }
}

xt::xarray<bool> NotRegion::isin(const xt::xarray<double>& coords) const {
    return !region_->isin(coords);
}

std::array<double, 6> NotRegion::boundary() const {
    return {-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
}

double NotRegion::volume() const {
    return std::numeric_limits<double>::infinity();
}

} // namespace molcpp
