#pragma once
#include <memory>
#include <vector>
#include "molcpp/types.hpp"

namespace molcpp {

class Region {
public:
    virtual ~Region() = default;

    // Membership query.
    // Input: XYZ with shape (3) or (N,3).
    // Output: boolean array with shape ()/(1) for single, or (N) for batch.
    virtual xt::xarray<bool> isIn(const XYZ& points) const = 0;

    // Finite volume (if applicable). For unbounded, return NaN or throw.
    virtual double getVolume() const = 0;
};

/**
 * @brief Region representing a parallelepiped (triclinic box)
 */
class ParallelepipedRegion : public Region {
public:
    /**
     * @brief Construct from triclinic cell matrix H and origin O
     * @param matrix 3x3 matrix where columns are lattice vectors
     * @param origin Origin point of the parallelepiped
     */
    ParallelepipedRegion(const Mat3<float>& matrix, const Vec3<float>& origin);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

    const Mat3<float>& getMatrix() const noexcept { return _matrix; }
    const Vec3<float>& getOrigin() const noexcept { return _origin; }

private:
    Mat3<float> _matrix;  // H (columns a,b,c), triclinic-supported
    Vec3<float> _origin;  // O
};

/**
 * @brief Region representing a cube
 */
class CubeRegion : public Region {
public:
    /**
     * @brief Construct from lower corner and edge length
     * @param lower_corner Lower corner point (x_min, y_min, z_min)
     * @param edge_length Edge length of the cube
     */
    CubeRegion(const Vec3<float>& lower_corner, float edge_length);

    /**
     * @brief Construct from lower corner and edge lengths
     * @param lower_corner Lower corner point (x_min, y_min, z_min)
     * @param edge_lengths Edge lengths (dx, dy, dz)
     */
    CubeRegion(const Vec3<float>& lower_corner, const Vec3<float>& edge_lengths);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

    const Vec3<float>& getLowerCorner() const noexcept { return _lower_corner; }
    const Vec3<float>& getEdgeLengths() const noexcept { return _edge_lengths; }

private:
    Vec3<float> _lower_corner;
    Vec3<float> _edge_lengths;
    static constexpr float TOLERANCE = 1e-6f;
};

/**
 * @brief Region representing a sphere
 */
class SphereRegion : public Region {
public:
    /**
     * @brief Construct from center and radius
     * @param center Center point of the sphere
     * @param radius Radius of the sphere
     */
    SphereRegion(const Vec3<float>& center, float radius);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

    const Vec3<float>& getCenter() const noexcept { return _center; }
    float getRadius() const noexcept { return _radius; }

private:
    Vec3<float> _center;
    float _radius;
    static constexpr float TOLERANCE = 1e-6f;
};

/**
 * @brief Region representing a cylinder
 */
class CylinderRegion : public Region {
public:
    /**
     * @brief Construct from two end points and radius
     * @param end1 First end point of the cylinder
     * @param end2 Second end point of the cylinder
     * @param radius Radius of the cylinder
     */
    CylinderRegion(const Vec3<float>& end1, const Vec3<float>& end2, float radius);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

    const Vec3<float>& getEnd1() const noexcept { return _end1; }
    const Vec3<float>& getEnd2() const noexcept { return _end2; }
    float getRadius() const noexcept { return _radius; }
    float getHeight() const noexcept { return _height; }

private:
    Vec3<float> _end1;
    Vec3<float> _end2;
    float _radius;
    float _height;
    Vec3<float> _axis;  // Normalized axis vector
    static constexpr float TOLERANCE = 1e-6f;
};

/**
 * @brief Region representing a plane with thickness
 */
class PlaneRegion : public Region {
public:
    /**
     * @brief Construct from a point on the plane, normal vector, and thickness
     * @param point A point on the plane
     * @param normal Normal vector to the plane
     * @param thickness Thickness of the region around the plane
     */
    PlaneRegion(const Vec3<float>& point, const Vec3<float>& normal, float thickness);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

    const Vec3<float>& getPoint() const noexcept { return _point; }
    const Vec3<float>& getNormal() const noexcept { return _normal; }
    float getThickness() const noexcept { return _thickness; }

private:
    Vec3<float> _point;
    Vec3<float> _normal;  // Normalized
    float _thickness;
    static constexpr float TOLERANCE = 1e-6f;
};

/**
 * @brief Region representing the intersection of multiple regions
 */
class IntersectionRegion : public Region {
public:
    /**
     * @brief Construct from a vector of regions
     * @param regions Vector of regions to intersect
     */
    IntersectionRegion(std::vector<std::shared_ptr<Region>> regions);

    /**
     * @brief Construct from two regions
     * @param region1 First region
     * @param region2 Second region
     */
    IntersectionRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

private:
    std::vector<std::shared_ptr<Region>> _regions;
};

/**
 * @brief Region representing the union of multiple regions
 */
class UnionRegion : public Region {
public:
    /**
     * @brief Construct from a vector of regions
     * @param regions Vector of regions to union
     */
    UnionRegion(std::vector<std::shared_ptr<Region>> regions);

    /**
     * @brief Construct from two regions
     * @param region1 First region
     * @param region2 Second region
     */
    UnionRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

private:
    std::vector<std::shared_ptr<Region>> _regions;
};

/**
 * @brief Region representing the complement of another region
 */
class ComplementRegion : public Region {
public:
    /**
     * @brief Construct from a region to complement
     * @param region Region to complement
     */
    ComplementRegion(std::shared_ptr<Region> region);

    xt::xarray<bool> isIn(const XYZ& points) const override;
    double getVolume() const override;

private:
    std::shared_ptr<Region> _region;
};

} // namespace molcpp
