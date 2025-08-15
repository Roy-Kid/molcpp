#pragma once

#include <array>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <xtensor/containers/xarray.hpp>
#include <xtensor/views/xview.hpp>
#include <xtensor/core/xmath.hpp>
#include "molcpp/types.hpp"
#include "molcpp/spatial/boundary.hpp"

namespace molcpp {

/**
 * @brief Abstract base class for spatial regions
 * 
 * Used to determine if particle coordinates satisfy geometric constraints.
 * Primarily used in molecular packing tools (molpack).
 */
class Region {
public:
    virtual ~Region() = default;

    /**
     * @brief Check which particles are inside this region
     * @param coords Array of shape (n, 3) containing particle positions
     * @return Boolean mask of shape (n,) indicating which points are inside
     */
    virtual xt::xarray<bool> isin(const xt::xarray<double>& coords) const = 0;

    /**
     * @brief Get the bounding box of this region
     * @return Array containing {xlo, xhi, ylo, yhi, zlo, zhi}
     * Used for coarse spatial filtering (e.g., octree construction)
     */
    virtual std::array<double, 6> boundary() const = 0;

    /**
     * @brief Get the volume of this region (if computable)
     * @return Volume of the region, or NaN if not computable
     */
    virtual double volume() const = 0;

    /**
     * @brief Check if this region can work with a boundary condition
     * @param boundary Boundary condition to check compatibility with
     * @return true if compatible, false otherwise
     */
    virtual bool is_compatible_with(const Boundary& boundary) const {
        // Default implementation: regions are compatible with all boundaries
        (void)boundary; // Suppress unused parameter warning
        return true;
    }

protected:
    // Numerical tolerance for floating point comparisons
    static constexpr double TOLERANCE = 1e-6;
};

/**
 * @brief Region representing the interior of a cube/rectangular box
 */
class InsideCube : public Region {
public:
    /**
     * @brief Construct a cube region
     * @param lo Lower corner of the cube
     * @param L Edge length of the cube (scalar for cube, Vec3 for box)
     */
    InsideCube(const Vec3& lo, double L);
    
    /**
     * @brief Construct a rectangular box region
     * @param lo Lower corner of the box
     * @param lengths Edge lengths [Lx, Ly, Lz]
     */
    InsideCube(const Vec3& lo, const Vec3& lengths);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const Vec3& get_lower_corner() const { return lo_; }
    const Vec3& get_lengths() const { return lengths_; }

private:
    Vec3 lo_;      ///< Lower corner coordinates
    Vec3 lengths_; ///< Edge lengths
};

/**
 * @brief Region representing the interior of a sphere
 */
class InsideSphere : public Region {
public:
    /**
     * @brief Construct a sphere region
     * @param center Center of the sphere
     * @param R Radius of the sphere
     */
    InsideSphere(const Vec3& center, double R);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const Vec3& get_center() const { return center_; }
    double get_radius() const { return R_; }

private:
    Vec3 center_; ///< Center coordinates
    double R_;    ///< Radius
};

/**
 * @brief Region representing the interior of a cylinder
 */
class InsideCylinder : public Region {
public:
    /**
     * @brief Construct a cylinder region
     * @param center1 Center of one circular end
     * @param center2 Center of the other circular end
     * @param radius Radius of the cylinder
     */
    InsideCylinder(const Vec3& center1, const Vec3& center2, double radius);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const Vec3& get_center1() const { return center1_; }
    const Vec3& get_center2() const { return center2_; }
    double get_radius() const { return radius_; }
    double get_height() const;

private:
    Vec3 center1_; ///< Center of first end
    Vec3 center2_; ///< Center of second end
    double radius_; ///< Radius
    Vec3 axis_;     ///< Normalized axis vector
    double height_; ///< Height (distance between centers)
};

/**
 * @brief Region representing points within a certain distance from a plane
 */
class NearPlane : public Region {
public:
    /**
     * @brief Construct a plane region
     * @param point A point on the plane
     * @param normal Normal vector to the plane (will be normalized)
     * @param thickness Half-thickness of the slab around the plane
     */
    NearPlane(const Vec3& point, const Vec3& normal, double thickness);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const Vec3& get_point() const { return point_; }
    const Vec3& get_normal() const { return normal_; }
    double get_thickness() const { return thickness_; }

private:
    Vec3 point_;      ///< Point on the plane
    Vec3 normal_;     ///< Unit normal vector
    double thickness_; ///< Half-thickness of slab
};

/**
 * @brief Boolean AND combination of multiple regions
 * All sub-regions must be true for isin to return true
 */
class AndRegion : public Region {
public:
    /**
     * @brief Construct AND region from vector of regions
     * @param regions Vector of shared pointers to regions
     */
    explicit AndRegion(std::vector<std::shared_ptr<Region>> regions);

    /**
     * @brief Construct AND region from two regions
     */
    AndRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const std::vector<std::shared_ptr<Region>>& get_regions() const { return regions_; }

private:
    std::vector<std::shared_ptr<Region>> regions_;
};

/**
 * @brief Boolean OR combination of multiple regions
 * At least one sub-region must be true for isin to return true
 */
class OrRegion : public Region {
public:
    /**
     * @brief Construct OR region from vector of regions
     * @param regions Vector of shared pointers to regions
     */
    explicit OrRegion(std::vector<std::shared_ptr<Region>> regions);

    /**
     * @brief Construct OR region from two regions
     */
    OrRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const std::vector<std::shared_ptr<Region>>& get_regions() const { return regions_; }

private:
    std::vector<std::shared_ptr<Region>> regions_;
};

/**
 * @brief Boolean NOT of a region
 * Returns the logical negation of the wrapped region
 */
class NotRegion : public Region {
public:
    /**
     * @brief Construct NOT region
     * @param region The region to negate
     */
    explicit NotRegion(std::shared_ptr<Region> region);

    xt::xarray<bool> isin(const xt::xarray<double>& coords) const override;
    std::array<double, 6> boundary() const override;
    double volume() const override;

    const std::shared_ptr<Region>& get_region() const { return region_; }

private:
    std::shared_ptr<Region> region_;
};

} // namespace molcpp
