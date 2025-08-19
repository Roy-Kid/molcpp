#include "molcpp/spatial/boundary.hpp"
#include "molcpp/spatial/box.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xmath.hpp>
#include <xtensor/views/xview.hpp>

namespace molcpp {

using XYZ = xt::xarray<float>;

PeriodicBoundary::PeriodicBoundary(bool px, bool py, bool pz) 
    : _px(px), _py(py), _pz(pz) {}

XYZ PeriodicBoundary::wrap(const Box& box, const XYZ& points) const {
    // Convert to fractional coordinates
    XYZ frac = box.toFrac(points);
    
    // Apply periodic wrapping only on periodic axes
    // Create a copy to avoid modifying the input
    XYZ wrapped_frac = frac;
    
    if (_px) {
        for (size_t i = 0; i < wrapped_frac.shape()[0]; ++i) {
            wrapped_frac(i, 0) = wrapped_frac(i, 0) - std::floor(wrapped_frac(i, 0));
        }
    }
    if (_py) {
        for (size_t i = 0; i < wrapped_frac.shape()[0]; ++i) {
            wrapped_frac(i, 1) = wrapped_frac(i, 1) - std::floor(wrapped_frac(i, 1));
        }
    }
    if (_pz) {
        for (size_t i = 0; i < wrapped_frac.shape()[0]; ++i) {
            wrapped_frac(i, 2) = wrapped_frac(i, 2) - std::floor(wrapped_frac(i, 2));
        }
    }
    
    // Convert back to Cartesian coordinates
    return box.toCart(wrapped_frac);
}

XYZ PeriodicBoundary::delta(const Box& box, const XYZ& a, const XYZ& b, bool minimumImage) const {
    if (!minimumImage) {
        return b - a;
    }
    
    // Convert both points to fractional coordinates
    XYZ frac_a = box.toFrac(a);
    XYZ frac_b = box.toFrac(b);
    
    // Calculate fractional displacement
    XYZ frac_delta = frac_b - frac_a;
    
    // Apply minimum image convention only on periodic axes
    if (_px) {
        for (size_t i = 0; i < frac_delta.shape()[0]; ++i) {
            frac_delta(i, 0) = frac_delta(i, 0) - std::round(frac_delta(i, 0));
        }
    }
    if (_py) {
        for (size_t i = 0; i < frac_delta.shape()[0]; ++i) {
            frac_delta(i, 1) = frac_delta(i, 1) - std::round(frac_delta(i, 1));
        }
    }
    if (_pz) {
        for (size_t i = 0; i < frac_delta.shape()[0]; ++i) {
            frac_delta(i, 2) = frac_delta(i, 2) - std::round(frac_delta(i, 2));
        }
    }
    
    // Convert back to Cartesian coordinates
    return box.toCart(frac_delta);
}

} // namespace molcpp
