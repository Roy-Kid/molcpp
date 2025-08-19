#pragma once
#include <memory>
#include "molcpp/types.hpp"
#include "molcpp/spatial/region.hpp"

namespace molcpp {

class Boundary; // forward declaration

class Box : public Region {
public:
    // Construct from triclinic cell matrix H and origin O.
    // pbc flags per axis (true = periodic).
    Box(const Mat3<float>& matrix, const Vec3<float>& origin, const Vec3<bool>& pbc);

    // Factories
    static Box cube(float length, const Vec3<float>& origin, const Vec3<bool>& pbc);
    static Box orthorhombic(const Vec3<float>& lengths, const Vec3<float>& origin, const Vec3<bool>& pbc);

    // Accessors
    const Mat3<float>& matrix() const noexcept { return _region.getMatrix(); }
    const Vec3<float>& origin() const noexcept { return _region.getOrigin(); }
    const Vec3<bool>& pbc() const noexcept { return _pbc; }

    // Coordinate transforms (XYZ with shape (3) or (N,3))
    XYZ toFrac(const XYZ& cart) const;  // frac = H^{-1} @ (cart - origin)
    XYZ toCart(const XYZ& frac) const;  // cart = origin + H @ frac

    // Region override (delegates to _region)
    xt::xarray<bool> isIn(const XYZ& points) const override;

    // Cell volume (|det(H)|)
    double getVolume() const override;

    // Boundary-aware ops (vectorized)
    XYZ wrap(const XYZ& points) const;  // wrap into primary cell
    XYZ delta(const XYZ& a, const XYZ& b, bool minimumImage = true) const;

private:
    Vec3<bool> _pbc;                // per-axis periodic mask
    ParallelepipedRegion _region;   // geometric region
    std::unique_ptr<Boundary> _boundary; // chosen from _pbc
};

} // namespace molcpp