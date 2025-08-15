#include "molcpp/spatial/boundary.hpp"
#include <stdexcept>
#include <limits>
#include <cmath>
#include <xtensor/core/xmath.hpp>
#include <xtensor/views/xview.hpp>
#include <xtensor-blas/xlinalg.hpp>

namespace molcpp {

// FreeBoundary implementation
auto FreeBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
    // Free boundary: no wrapping, return coordinates as-is
    return coords;
}

auto FreeBoundary::minimum_image(const xt::xarray<double>& r1, 
                                const xt::xarray<double>& r2) const -> xt::xarray<double> {
    // Free boundary: simple difference
    return r2 - r1;
}

auto FreeBoundary::get_bounds() const -> std::array<double, 6> {
    // Free boundary: infinite bounds
    constexpr double inf = std::numeric_limits<double>::max();
    return {-inf, inf, -inf, inf, -inf, inf};
}

auto FreeBoundary::is_periodic() const -> std::array<bool, 3> {
    return {false, false, false};
}

// OrthogonalBoundary implementation
OrthogonalBoundary::OrthogonalBoundary(const Vec3& box_lengths, 
                                      const std::array<bool, 3>& periodic)
    : box_lengths_(box_lengths), periodic_(periodic) {
    
    // Validate box lengths
    for (size_t i = 0; i < 3; ++i) {
        if (box_lengths_[i] <= 0) {
            throw std::invalid_argument("Box lengths must be positive");
        }
    }
}

auto OrthogonalBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Create masks and box lengths for vectorized operations
    xt::xarray<bool> periodic_mask = {periodic_[0], periodic_[1], periodic_[2]};
    xt::xarray<double> box_lengths = {box_lengths_[0], box_lengths_[1], box_lengths_[2]};

    // Vectorized periodic wrapping: coords - box_lengths * floor(coords / box_lengths)
    auto wrapped_coords = coords - box_lengths * xt::floor(coords / box_lengths);
    
    // Only apply wrapping to periodic dimensions
    auto result = xt::where(periodic_mask, wrapped_coords, coords);
    
    return result;
}

auto OrthogonalBoundary::minimum_image(const xt::xarray<double>& r1, 
                                      const xt::xarray<double>& r2) const -> xt::xarray<double> {
    xt::xarray<double> dr = r2 - r1;
    
    // Create masks for periodic dimensions
    xt::xarray<bool> periodic_mask = {periodic_[0], periodic_[1], periodic_[2]};
    xt::xarray<double> box_lengths = {box_lengths_[0], box_lengths_[1], box_lengths_[2]};
    
    if (dr.dimension() == 1 && dr.shape(0) == 3) {
        // Single vector case - vectorized operation
        auto wrapped = xt::where(periodic_mask, 
                                dr - box_lengths * xt::round(dr / box_lengths),
                                dr);
        return wrapped;
    } else if (dr.dimension() == 2 && dr.shape(1) == 3) {
        // Multiple vectors case - broadcast operation
        auto wrapped = xt::where(periodic_mask,
                                dr - box_lengths * xt::round(dr / box_lengths),
                                dr);
        return wrapped;
    } else {
        throw std::invalid_argument("Invalid shape for distance vector");
    }
}

auto OrthogonalBoundary::get_bounds() const -> std::array<double, 6> {
    return {0.0, box_lengths_[0], 0.0, box_lengths_[1], 0.0, box_lengths_[2]};
}

auto OrthogonalBoundary::is_periodic() const -> std::array<bool, 3> {
    return periodic_;
}

// TriclinicBoundary implementation
TriclinicBoundary::TriclinicBoundary(const Mat3& lattice_matrix,
                                    const std::array<bool, 3>& periodic)
    : lattice_matrix_(lattice_matrix), periodic_(periodic) {
    
    // Validate matrix
    if (lattice_matrix_.shape()[0] != 3 || lattice_matrix_.shape()[1] != 3) {
        throw std::invalid_argument("Lattice matrix must be 3x3");
    }
    
    // Compute inverse matrix
    double det = xt::linalg::det(lattice_matrix_);
    if (std::abs(det) < TOLERANCE) {
        throw std::invalid_argument("Lattice matrix is singular");
    }
    
    inverse_matrix_ = xt::linalg::inv(lattice_matrix_);
    compute_bounds();
}

auto TriclinicBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    // Convert all coordinates to fractional at once
    auto r_frac = xt::linalg::dot(coords, xt::transpose(inverse_matrix_));
    
    // Apply periodic wrapping in fractional space (vectorized)
    xt::xarray<bool> periodic_mask = {periodic_[0], periodic_[1], periodic_[2]};
    auto r_frac_wrapped = xt::where(periodic_mask,
                                   r_frac - xt::floor(r_frac),
                                   r_frac);
    
    // Convert back to Cartesian coordinates
    auto wrapped = xt::linalg::dot(r_frac_wrapped, xt::transpose(lattice_matrix_));
    
    return wrapped;
}

auto TriclinicBoundary::minimum_image(const xt::xarray<double>& r1, 
                                     const xt::xarray<double>& r2) const -> xt::xarray<double> {
    xt::xarray<double> dr = r2 - r1;
    
    if (dr.dimension() == 1 && dr.shape(0) == 3) {
        // Single vector case
        auto dr_frac = xt::linalg::dot(dr, inverse_matrix_);
        
        // Apply minimum image in fractional space (vectorized)
        xt::xarray<bool> periodic_mask = {periodic_[0], periodic_[1], periodic_[2]};
        auto dr_frac_wrapped = xt::where(periodic_mask,
                                        dr_frac - xt::round(dr_frac),
                                        dr_frac);
        
        // Convert back to Cartesian
        return xt::linalg::dot(dr_frac_wrapped, lattice_matrix_);
        
    } else if (dr.dimension() == 2 && dr.shape(1) == 3) {
        // Multiple vectors case - fully vectorized
        auto dr_frac = xt::linalg::dot(dr, xt::transpose(inverse_matrix_));
        
        // Apply minimum image in fractional space
        xt::xarray<bool> periodic_mask = {periodic_[0], periodic_[1], periodic_[2]};
        auto dr_frac_wrapped = xt::where(periodic_mask,
                                        dr_frac - xt::round(dr_frac),
                                        dr_frac);
        
        // Convert back to Cartesian
        return xt::linalg::dot(dr_frac_wrapped, xt::transpose(lattice_matrix_));
    } else {
        throw std::invalid_argument("Invalid shape for distance vector");
    }
}

auto TriclinicBoundary::get_bounds() const -> std::array<double, 6> {
    return bounds_;
}

auto TriclinicBoundary::is_periodic() const -> std::array<bool, 3> {
    return periodic_;
}

void TriclinicBoundary::compute_bounds() {
    // Find the bounding box of the parallelepiped
    // Check all 8 corners of the unit cell
    std::array<Vec3, 8> corners = {{
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
    }};

    double xmin = std::numeric_limits<double>::max();
    double xmax = std::numeric_limits<double>::lowest();
    double ymin = std::numeric_limits<double>::max();
    double ymax = std::numeric_limits<double>::lowest();
    double zmin = std::numeric_limits<double>::max();
    double zmax = std::numeric_limits<double>::lowest();

    for (const auto& corner_frac : corners) {
        // Convert fractional to Cartesian
        auto corner_cart = xt::linalg::dot(corner_frac, lattice_matrix_);
        
        xmin = std::min(xmin, corner_cart(0));
        xmax = std::max(xmax, corner_cart(0));
        ymin = std::min(ymin, corner_cart(1));
        ymax = std::max(ymax, corner_cart(1));
        zmin = std::min(zmin, corner_cart(2));
        zmax = std::max(zmax, corner_cart(2));
    }

    bounds_ = {xmin, xmax, ymin, ymax, zmin, zmax};
}

// SphericalBoundary implementation
SphericalBoundary::SphericalBoundary(const Vec3& center, double radius, bool periodic)
    : center_(center), radius_(radius), periodic_(periodic) {
    
    if (radius <= 0) {
        throw std::invalid_argument("Radius must be positive");
    }
}

auto SphericalBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
    if (coords.dimension() != 2 || coords.shape(1) != 3) {
        throw std::invalid_argument("Coordinates must have shape (n, 3)");
    }

    if (!periodic_) {
        // No wrapping for non-periodic spherical boundary
        return coords;
    }

    auto wrapped = coords;
    const size_t n_particles = coords.shape(0);

    for (size_t i = 0; i < n_particles; ++i) {
        // Calculate distance from center
        double dx = coords(i, 0) - center_[0];
        double dy = coords(i, 1) - center_[1];
        double dz = coords(i, 2) - center_[2];
        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        if (r > radius_) {
            // Wrap to opposite side of sphere
            double scale = (2 * radius_ - r) / r;
            wrapped(i, 0) = center_[0] + dx * scale;
            wrapped(i, 1) = center_[1] + dy * scale;
            wrapped(i, 2) = center_[2] + dz * scale;
        }
    }

    return wrapped;
}

auto SphericalBoundary::minimum_image(const xt::xarray<double>& r1, 
                                     const xt::xarray<double>& r2) const -> xt::xarray<double> {
    // For spherical boundary, minimum image is complex
    // This is a simplified implementation
    auto dr = r2 - r1;
    
    if (!periodic_) {
        return dr;
    }
    
    // For periodic spherical boundary, we would need to consider
    // multiple paths around the sphere. This is quite complex.
    // For now, return simple difference
    return dr;
}

auto SphericalBoundary::get_bounds() const -> std::array<double, 6> {
    return {
        center_[0] - radius_, center_[0] + radius_,
        center_[1] - radius_, center_[1] + radius_,
        center_[2] - radius_, center_[2] + radius_
    };
}

auto SphericalBoundary::is_periodic() const -> std::array<bool, 3> {
    // Spherical periodicity is more complex than Cartesian
    return {periodic_, periodic_, periodic_};
}

} // namespace molcpp
