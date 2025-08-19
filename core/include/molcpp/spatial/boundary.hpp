
#pragma once

#include "molcpp/types.hpp"
#include <memory>

namespace molcpp {

class Box; // forward declaration

class Boundary {
public:
    virtual ~Boundary() = default;

    // Wrap points into primary cell
    virtual XYZ wrap(const Box& box, const XYZ& points) const = 0;

    // Calculate vector between two points with minimum image convention
    virtual XYZ delta(const Box& box, const XYZ& a, const XYZ& b, bool minimumImage = true) const = 0;
};

/**
 * @brief Open boundary conditions (no wrapping)
 */
class OpenBoundary : public Boundary {
public:
    XYZ wrap(const Box& box, const XYZ& points) const override;
    XYZ delta(const Box& box, const XYZ& a, const XYZ& b, bool minimumImage = true) const override;
};

/**
 * @brief Periodic boundary conditions
 */
class PeriodicBoundary : public Boundary {
public:
    PeriodicBoundary(bool px, bool py, bool pz);
    
    XYZ wrap(const Box& box, const XYZ& points) const override;
    XYZ delta(const Box& box, const XYZ& a, const XYZ& b, bool minimumImage = true) const override;

private:
    bool _px, _py, _pz;  // periodic flags per axis
};

} // namespace molcpp