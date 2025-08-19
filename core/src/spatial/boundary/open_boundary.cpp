#include "molcpp/spatial/boundary.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/types.hpp"
#include <xtensor/containers/xarray.hpp>

namespace molcpp {

XYZ OpenBoundary::wrap(const Box& box, const XYZ& points) const {
    // No wrapping - return points as-is
    return points;
}

XYZ OpenBoundary::delta(const Box& box, const XYZ& a, const XYZ& b, bool minimumImage) const {
    // No minimum image convention - just return b - a
    return b - a;
}

} // namespace molcpp
