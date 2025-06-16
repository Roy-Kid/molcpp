
#pragma once

#include <array>
#include <memory>
#include <vector>
#include <xtensor/containers/xarray.hpp>
#include "molcpp/types.hpp"
#include "molcpp/exports.h"

namespace molcpp {

/**
 * @brief Abstract base class for spatial boundaries
 * 
 * Defines boundary conditions for spatial regions, including periodic
 * boundary conditions commonly used in molecular simulations.
 */
class Boundary
{
  public:
    /// Default constructor
    Boundary() = default;

    virtual ~Boundary() = default;

    /// Declare copy constructor
    Boundary(const Boundary &) = default;

    /// Declare copy assignment operator
    auto operator=(const Boundary &) -> Boundary & = default;

    /// Declare move constructor
    Boundary(Boundary &&) = default;

    /// Declare move assignment operator
    auto operator=(Boundary &&) -> Boundary & = default;

    /**
     * @brief Apply boundary conditions to coordinates
     * @param coords Array of shape (n, 3) containing particle positions
     * @return Wrapped coordinates after applying boundary conditions
     */
    virtual auto wrap(const xt::xarray<double> &coords) const -> xt::xarray<double> = 0;

    /**
     * @brief Calculate minimum image distance between two points
     * @param r1 First point coordinates (3D)
     * @param r2 Second point coordinates (3D) 
     * @return Minimum image distance vector from r1 to r2
     */
    virtual auto minimum_image(const xt::xarray<double>& r1, 
                              const xt::xarray<double>& r2) const -> xt::xarray<double> = 0;

    /**
     * @brief Get the bounding box of the boundary
     * @return Array containing {xlo, xhi, ylo, yhi, zlo, zhi}
     */
    virtual auto get_bounds() const -> std::array<double, 6> = 0;

    /**
     * @brief Check if boundary conditions are periodic
     * @return Array of booleans for [x, y, z] periodicity
     */
    virtual auto is_periodic() const -> std::array<bool, 3> = 0;

protected:
    // Numerical tolerance for floating point comparisons
    static constexpr double TOLERANCE = 1e-10;
};

// /**
//  * @brief Free boundary (no wrapping)
//  * Represents infinite space with no boundary conditions
//  */
// class FreeBoundary : public Boundary {
// public:
//     FreeBoundary() = default;

//     auto wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> override;
//     auto minimum_image(const xt::xarray<double>& r1, 
//                       const xt::xarray<double>& r2) const -> xt::xarray<double> override;
//     auto get_bounds() const -> std::array<double, 6> override;
//     auto is_periodic() const -> std::array<bool, 3> override;
// };

// /**
//  * @brief Orthogonal periodic boundary
//  * Represents a rectangular simulation box with periodic boundary conditions
//  */
// class OrthogonalBoundary : public Boundary {
// public:
//     /**
//      * @brief Construct orthogonal boundary
//      * @param box_lengths Box dimensions [Lx, Ly, Lz]
//      * @param periodic Periodicity in each dimension [px, py, pz]
//      */
//     OrthogonalBoundary(const Vec3& box_lengths, 
//                       const std::array<bool, 3>& periodic = {true, true, true});

//     auto wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> override;
//     auto minimum_image(const xt::xarray<double>& r1, 
//                       const xt::xarray<double>& r2) const -> xt::xarray<double> override;
//     auto get_bounds() const -> std::array<double, 6> override;
//     auto is_periodic() const -> std::array<bool, 3> override;

//     auto get_box_lengths() const -> const Vec3& { return box_lengths_; }

// private:
//     Vec3 box_lengths_;              ///< Box dimensions
//     std::array<bool, 3> periodic_;  ///< Periodicity flags
// };

// /**
//  * @brief Triclinic periodic boundary
//  * Represents a general parallelepiped simulation box
//  */
// class TriclinicBoundary : public Boundary {
// public:
//     /**
//      * @brief Construct triclinic boundary from lattice vectors
//      * @param lattice_matrix 3x3 matrix where rows are lattice vectors
//      * @param periodic Periodicity in each dimension [px, py, pz]
//      */
//     TriclinicBoundary(const Mat3& lattice_matrix,
//                      const std::array<bool, 3>& periodic = {true, true, true});

//     auto wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> override;
//     auto minimum_image(const xt::xarray<double>& r1, 
//                       const xt::xarray<double>& r2) const -> xt::xarray<double> override;
//     auto get_bounds() const -> std::array<double, 6> override;
//     auto is_periodic() const -> std::array<bool, 3> override;

//     auto get_lattice_matrix() const -> const Mat3& { return lattice_matrix_; }
//     auto get_inverse_matrix() const -> const Mat3& { return inverse_matrix_; }

// private:
//     Mat3 lattice_matrix_;           ///< Lattice vector matrix
//     Mat3 inverse_matrix_;           ///< Inverse lattice matrix
//     std::array<bool, 3> periodic_;  ///< Periodicity flags
//     std::array<double, 6> bounds_;  ///< Cached bounding box

//     void compute_bounds();
// };

// /**
//  * @brief Spherical boundary
//  * Represents a spherical boundary with optional wrapping
//  */
// class SphericalBoundary : public Boundary {
// public:
//     /**
//      * @brief Construct spherical boundary
//      * @param center Center of the sphere
//      * @param radius Radius of the sphere
//      * @param periodic Whether to wrap coordinates (default: false)
//      */
//     SphericalBoundary(const Vec3& center, double radius, bool periodic = false);

//     auto wrap(const xt::xarray<double>& coords) const -> xt::xarray<double> override;
//     auto minimum_image(const xt::xarray<double>& r1, 
//                       const xt::xarray<double>& r2) const -> xt::xarray<double> override;
//     auto get_bounds() const -> std::array<double, 6> override;
//     auto is_periodic() const -> std::array<bool, 3> override;

//     auto get_center() const -> const Vec3& { return center_; }
//     auto get_radius() const -> double { return radius_; }

// private:
//     Vec3 center_;    ///< Center of sphere
//     double radius_;  ///< Radius of sphere
//     bool periodic_;  ///< Whether wrapping is enabled
// };

} // namespace molcpp