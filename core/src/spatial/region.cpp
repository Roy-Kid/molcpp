#include "molcpp/spatial/region.hpp"
#include <stdexcept>
#include <limits>
#include <cmath>
#include <xtensor/core/xmath.hpp>
#include <xtensor/views/xview.hpp>

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

bool InsideCube::isin(const xt::xarray<double>& coords) const {
    // Check shape: should be (n, 3)
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Vectorized (numpy-style) check using xtensor
    auto lower = lo_ - TOLERANCE;
    auto upper = lo_ + lengths_ + TOLERANCE;
    auto coords_view = xt::view(coords, xt::all(), xt::range(0, 3));
    auto ge_lower = coords_view >= lower;
    auto le_upper = coords_view <= upper;
    auto in_bounds = ge_lower && le_upper;
    
    // Check if all particles and all dimensions are within bounds
    return xt::all(in_bounds);  // Check if all elements are true
}

xt::xarray<bool> InsideCube::mask(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    xt::xarray<bool> result = xt::ones<bool>({n_particles});
    
    for (size_t i = 0; i < n_particles; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            double coord = coords(i, j);
            double lower_bound = lo_[j] - TOLERANCE;
            double upper_bound = lo_[j] + lengths_[j] + TOLERANCE;
            
            if (coord < lower_bound || coord > upper_bound) {
                result(i) = false;
                break;
            }
        }
    }
    
    return result;
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

bool InsideSphere::isin(const xt::xarray<double>& coords) const {
    // Check shape: should be (n, 3)
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    const double R_squared = R_ * R_;
    
    // Check each particle
    for (size_t i = 0; i < n_particles; ++i) {
        double dist_squared = 0.0;
        for (size_t j = 0; j < 3; ++j) {
            double diff = coords(i, j) - center_[j];
            dist_squared += diff * diff;
        }
        
        if (dist_squared > R_squared + TOLERANCE) {
            return false;
        }
    }
    
    return true;
}

xt::xarray<bool> InsideSphere::mask(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    const double R_squared = R_ * R_;
    xt::xarray<bool> result = xt::zeros<bool>({n_particles});
    
    for (size_t i = 0; i < n_particles; ++i) {
        double dist_squared = 0.0;
        for (size_t j = 0; j < 3; ++j) {
            double diff = coords(i, j) - center_[j];
            dist_squared += diff * diff;
        }
        
        result(i) = (dist_squared <= R_squared + TOLERANCE);
    }
    
    return result;
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

bool InsideCylinder::isin(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    
    for (size_t i = 0; i < n_particles; ++i) {
        // Vector from center1 to point
        double px = coords(i, 0) - center1_[0];
        double py = coords(i, 1) - center1_[1];
        double pz = coords(i, 2) - center1_[2];
        
        // Project onto cylinder axis
        double proj_length = px * axis_[0] + py * axis_[1] + pz * axis_[2];
        
        // Check if projection is within cylinder height
        if (proj_length < -TOLERANCE || proj_length > height_ + TOLERANCE) {
            return false;
        }
        
        // Calculate perpendicular distance to axis
        double proj_x = proj_length * axis_[0];
        double proj_y = proj_length * axis_[1];
        double proj_z = proj_length * axis_[2];
        
        double perp_x = px - proj_x;
        double perp_y = py - proj_y;
        double perp_z = pz - proj_z;
        
        double perp_dist_sq = perp_x*perp_x + perp_y*perp_y + perp_z*perp_z;
        
        if (perp_dist_sq > radius_*radius_ + TOLERANCE) {
            return false;
        }
    }
    
    return true;
}

xt::xarray<bool> InsideCylinder::mask(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    xt::xarray<bool> result = xt::zeros<bool>({n_particles});
    
    for (size_t i = 0; i < n_particles; ++i) {
        // Vector from center1 to point
        double px = coords(i, 0) - center1_[0];
        double py = coords(i, 1) - center1_[1];
        double pz = coords(i, 2) - center1_[2];
        
        // Project onto cylinder axis
        double proj_length = px * axis_[0] + py * axis_[1] + pz * axis_[2];
        
        // Check if projection is within cylinder height
        if (proj_length >= -TOLERANCE && proj_length <= height_ + TOLERANCE) {
            // Calculate perpendicular distance to axis
            double proj_x = proj_length * axis_[0];
            double proj_y = proj_length * axis_[1];
            double proj_z = proj_length * axis_[2];
            
            double perp_x = px - proj_x;
            double perp_y = py - proj_y;
            double perp_z = pz - proj_z;
            
            double perp_dist_sq = perp_x*perp_x + perp_y*perp_y + perp_z*perp_z;
            
            result(i) = (perp_dist_sq <= radius_*radius_ + TOLERANCE);
        }
    }
    
    return result;
}

std::array<double, 6> InsideCylinder::boundary() const {
    // Find bounding box of cylinder
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
    : point_(point), thickness_(thickness) {
    
    if (thickness <= 0) {
        throw std::invalid_argument("Plane thickness must be positive");
    }
    
    // Normalize the normal vector
    double norm = std::sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
    if (norm < TOLERANCE) {
        throw std::invalid_argument("Normal vector cannot be zero");
    }
    
    normal_[0] = normal[0] / norm;
    normal_[1] = normal[1] / norm;
    normal_[2] = normal[2] / norm;
}

bool NearPlane::isin(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    
    for (size_t i = 0; i < n_particles; ++i) {
        // Vector from plane point to particle
        double dx = coords(i, 0) - point_[0];
        double dy = coords(i, 1) - point_[1];
        double dz = coords(i, 2) - point_[2];
        
        // Distance to plane (signed)
        double dist = dx * normal_[0] + dy * normal_[1] + dz * normal_[2];
        
        if (std::abs(dist) > thickness_ + TOLERANCE) {
            return false;
        }
    }
    
    return true;
}

xt::xarray<bool> NearPlane::mask(const xt::xarray<double>& coords) const {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    const size_t n_particles = coords.shape(0);
    xt::xarray<bool> result = xt::zeros<bool>({n_particles});
    
    for (size_t i = 0; i < n_particles; ++i) {
        // Vector from plane point to particle
        double dx = coords(i, 0) - point_[0];
        double dy = coords(i, 1) - point_[1];
        double dz = coords(i, 2) - point_[2];
        
        // Distance to plane (signed)
        double dist = dx * normal_[0] + dy * normal_[1] + dz * normal_[2];
        
        result(i) = (std::abs(dist) <= thickness_ + TOLERANCE);
    }
    
    return result;
}

std::array<double, 6> NearPlane::boundary() const {
    // For infinite plane, return very large bounds
    constexpr double large = 1e10;
    return {-large, large, -large, large, -large, large};
}

double NearPlane::volume() const {
    // Infinite volume for plane slab
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

bool AndRegion::isin(const xt::xarray<double>& coords) const {
    // All regions must be true
    for (const auto& region : regions_) {
        if (!region->isin(coords)) {
            return false;
        }
    }
    return true;
}

xt::xarray<bool> AndRegion::mask(const xt::xarray<double>& coords) const {
    if (regions_.empty()) {
        throw std::runtime_error("AndRegion has no regions");
    }
    
    auto result = regions_[0]->mask(coords);
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        result = result && regions_[i]->mask(coords);
    }
    
    return result;
}

std::array<double, 6> AndRegion::boundary() const {
    // Intersection of all boundaries
    auto result = regions_[0]->boundary();
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        auto boundary = regions_[i]->boundary();
        
        // Intersection: max of lower bounds, min of upper bounds
        result[0] = std::max(result[0], boundary[0]); // xlo
        result[1] = std::min(result[1], boundary[1]); // xhi
        result[2] = std::max(result[2], boundary[2]); // ylo
        result[3] = std::min(result[3], boundary[3]); // yhi
        result[4] = std::max(result[4], boundary[4]); // zlo
        result[5] = std::min(result[5], boundary[5]); // zhi
    }
    
    return result;
}

double AndRegion::volume() const {
    // Volume of intersection is complex to compute exactly
    // Return NaN to indicate it's not easily computable
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

bool OrRegion::isin(const xt::xarray<double>& coords) const {
    // At least one region must be true
    for (const auto& region : regions_) {
        if (region->isin(coords)) {
            return true;
        }
    }
    return false;
}

xt::xarray<bool> OrRegion::mask(const xt::xarray<double>& coords) const {
    if (regions_.empty()) {
        throw std::runtime_error("OrRegion has no regions");
    }
    
    auto result = regions_[0]->mask(coords);
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        result = result || regions_[i]->mask(coords);
    }
    
    return result;
}

std::array<double, 6> OrRegion::boundary() const {
    // Union of all boundaries
    auto result = regions_[0]->boundary();
    
    for (size_t i = 1; i < regions_.size(); ++i) {
        auto boundary = regions_[i]->boundary();
        
        // Union: min of lower bounds, max of upper bounds
        result[0] = std::min(result[0], boundary[0]); // xlo
        result[1] = std::max(result[1], boundary[1]); // xhi
        result[2] = std::min(result[2], boundary[2]); // ylo
        result[3] = std::max(result[3], boundary[3]); // yhi
        result[4] = std::min(result[4], boundary[4]); // zlo
        result[5] = std::max(result[5], boundary[5]); // zhi
    }
    
    return result;
}

double OrRegion::volume() const {
    // Volume of union is complex to compute exactly
    // Return NaN to indicate it's not easily computable
    return std::numeric_limits<double>::quiet_NaN();
}

// NotRegion implementation
NotRegion::NotRegion(std::shared_ptr<Region> region) : region_(std::move(region)) {
    if (!region_) {
        throw std::invalid_argument("NotRegion requires a valid region");
    }
}

bool NotRegion::isin(const xt::xarray<double>& coords) const {
    // Logical negation of the wrapped region
    return !region_->isin(coords);
}

xt::xarray<bool> NotRegion::mask(const xt::xarray<double>& coords) const {
    return !region_->mask(coords);
}

std::array<double, 6> NotRegion::boundary() const {
    // For NOT operation, the boundary is essentially unbounded
    // We return a very large bounding box
    constexpr double inf = std::numeric_limits<double>::max();
    return {-inf, inf, -inf, inf, -inf, inf};
}

double NotRegion::volume() const {
    // Complement has infinite volume
    return std::numeric_limits<double>::infinity();
}

} // namespace molcpp
