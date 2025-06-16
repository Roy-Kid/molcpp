#include "molcpp/spatial/boundary.hpp"

namespace molcpp {
// Boundary is now a pure abstract class - no implementations needed

// All concrete implementations have been commented out
// Boundary functionality //     bounds_ = {xmin, xmax, ymin, ymax, zmin, zmax};
// }ow handled by the Box class

} // namespace molcpp

// auto OrthogonalBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
//     if (coords.dimension() != 2 || coords.shape(1) != 3) {
//         throw std::invalid_argument("Coordinates must have shape (n, 3)");
//     }

//     auto wrapped = coords;
//     const size_t n_particles = coords.shape(0);

//     for (size_t i = 0; i < n_particles; ++i) {
//         for (size_t j = 0; j < 3; ++j) {
//             if (periodic_[j]) {
//                 // Apply periodic wrapping
//                 double coord = coords(i, j);
//                 double box_length = box_lengths_[j];
                
//                 // Wrap to [0, box_length)
//                 wrapped(i, j) = coord - box_length * std::floor(coord / box_length);
//             }
//         }
//     }

//     return wrapped;
// }

// auto OrthogonalBoundary::minimum_image(const xt::xarray<double>& r1, 
//                                       const xt::xarray<double>& r2) const -> xt::xarray<double> {
//     auto dr = r2 - r1;
    
//     if (dr.dimension() == 1 && dr.shape(0) == 3) {
//         // Single vector case - create a new array to avoid modifying expression
//         xt::xarray<double> result = dr;
//         for (size_t j = 0; j < 3; ++j) {
//             if (periodic_[j]) {
//                 double box_length = box_lengths_[j];
//                 double half_box = box_length * 0.5;
                
//                 if (result(j) > half_box) {
//                     result(j) -= box_length;
//                 } else if (result(j) < -half_box) {
//                     result(j) += box_length;
//                 }
//             }
//         }
//         return result;
//     } else if (dr.dimension() == 2 && dr.shape(1) == 3) {
//         // Multiple vectors case
//         xt::xarray<double> result = dr;
//         const size_t n = result.shape(0);
//         for (size_t i = 0; i < n; ++i) {
//             for (size_t j = 0; j < 3; ++j) {
//                 if (periodic_[j]) {
//                     double box_length = box_lengths_[j];
//                     double half_box = box_length * 0.5;
                    
//                     if (result(i, j) > half_box) {
//                         result(i, j) -= box_length;
//                     } else if (result(i, j) < -half_box) {
//                         result(i, j) += box_length;
//                     }
//                 }
//             }
//         }
//         return result;
//     } else {
//         throw std::invalid_argument("Invalid shape for distance vector");
//     }
// }

// auto OrthogonalBoundary::get_bounds() const -> std::array<double, 6> {
//     return {0.0, box_lengths_[0], 0.0, box_lengths_[1], 0.0, box_lengths_[2]};
// }

// auto OrthogonalBoundary::is_periodic() const -> std::array<bool, 3> {
//     return periodic_;
// }

// // TriclinicBoundary implementation
// TriclinicBoundary::TriclinicBoundary(const Mat3& lattice_matrix,
//                                     const std::array<bool, 3>& periodic)
//     : lattice_matrix_(lattice_matrix), periodic_(periodic) {
    
//     // Validate matrix
//     if (lattice_matrix_.shape()[0] != 3 || lattice_matrix_.shape()[1] != 3) {
//         throw std::invalid_argument("Lattice matrix must be 3x3");
//     }
    
//     // Compute inverse matrix
//     double det = xt::linalg::det(lattice_matrix_);
//     if (std::abs(det) < TOLERANCE) {
//         throw std::invalid_argument("Lattice matrix is singular");
//     }
    
//     inverse_matrix_ = xt::linalg::inv(lattice_matrix_);
//     compute_bounds();
// }

// auto TriclinicBoundary::wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> {
//     if (coords.dimension() != 2 || coords.shape(1) != 3) {
//         throw std::invalid_argument("Coordinates must have shape (n, 3)");
//     }

//     auto wrapped = coords;
//     const size_t n_particles = coords.shape(0);

//     for (size_t i = 0; i < n_particles; ++i) {
//         // Convert to fractional coordinates
//         auto r_cart = xt::view(coords, i, xt::all());
//         auto r_frac = xt::linalg::dot(r_cart, inverse_matrix_);
        
//         // Apply periodic wrapping in fractional space
//         for (size_t j = 0; j < 3; ++j) {
//             if (periodic_[j]) {
//                 r_frac(j) = r_frac(j) - std::floor(r_frac(j));
//             }
//         }
        
//         // Convert back to Cartesian coordinates
//         auto r_wrapped = xt::linalg::dot(r_frac, lattice_matrix_);
//         xt::view(wrapped, i, xt::all()) = r_wrapped;
//     }

//     return wrapped;
// }

// auto TriclinicBoundary::minimum_image(const xt::xarray<double>& r1, 
//                                      const xt::xarray<double>& r2) const -> xt::xarray<double> {
//     auto dr = r2 - r1;
    
//     if (dr.dimension() == 1 && dr.shape(0) == 3) {
//         // Convert to fractional coordinates
//         xt::xarray<double> dr_copy = dr;  // Make a copy to avoid modifying expression
//         auto dr_frac = xt::linalg::dot(dr_copy, inverse_matrix_);
        
//         // Apply minimum image in fractional space
//         for (size_t j = 0; j < 3; ++j) {
//             if (periodic_[j]) {
//                 dr_frac(j) = dr_frac(j) - std::round(dr_frac(j));
//             }
//         }
        
//         // Convert back to Cartesian
//         return xt::linalg::dot(dr_frac, lattice_matrix_);
        
//     } else if (dr.dimension() == 2 && dr.shape(1) == 3) {
//         // Multiple vectors case
//         xt::xarray<double> result = dr;  // Make a copy
//         const size_t n = result.shape(0);
//         for (size_t i = 0; i < n; ++i) {
//             auto dr_i = xt::view(result, i, xt::all());
//             xt::xarray<double> dr_i_copy = dr_i;  // Copy the view
//             auto dr_frac = xt::linalg::dot(dr_i_copy, inverse_matrix_);
            
//             for (size_t j = 0; j < 3; ++j) {
//                 if (periodic_[j]) {
//                     dr_frac(j) = dr_frac(j) - std::round(dr_frac(j));
//                 }
//             }
            
//             xt::view(result, i, xt::all()) = xt::linalg::dot(dr_frac, lattice_matrix_);
//         }
//         return result;
//     } else {
//         throw std::invalid_argument("Invalid shape for distance vector");
//     }
// }

// auto TriclinicBoundary::get_bounds() const -> std::array<double, 6> {
//     return bounds_;
// }

// auto TriclinicBoundary::is_periodic() const -> std::array<bool, 3> {
//     return periodic_;
// }

// void TriclinicBoundary::compute_bounds() {
//     // Find the bounding box of the parallelepiped
//     // Check all 8 corners of the unit cell
//     std::array<Vec3, 8> corners = {{
//         {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
//         {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
//     }};

//     double xmin = std::numeric_limits<double>::max();
//     double xmax = std::numeric_limits<double>::lowest();
//     double ymin = std::numeric_limits<double>::max();
//     double ymax = std::numeric_limits<double>::lowest();
//     double zmin = std::numeric_limits<double>::max();
//     double zmax = std::numeric_limits<double>::lowest();

//     for (const auto& corner_frac : corners) {
//         // Convert fractional to Cartesian
//         auto corner_cart = xt::linalg::dot(corner_frac, lattice_matrix_);
        
//         xmin = std::min(xmin, corner_cart(0));
//         xmax = std::max(xmax, corner_cart(0));
//         ymin = std::min(ymin, corner_cart(1));
//         ymax = std::max(ymax, corner_cart(1));
//         zmin = std::min(zmin, corner_cart(2));
//         zmax = std::max(zmax, corner_cart(2));
//     }

//     bounds_ = {xmin, xmax, ymin, ymax, zmin, zmax};
// }

// } // namespace molcpp
