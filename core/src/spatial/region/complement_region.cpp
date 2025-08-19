#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <stdexcept>
#include <limits>

namespace molcpp {

using XYZ = xt::xarray<float>;

ComplementRegion::ComplementRegion(std::shared_ptr<Region> region) : _region(std::move(region)) {
    if (!_region) {
        throw std::invalid_argument("ComplementRegion requires a valid region");
    }
}

xt::xarray<bool> ComplementRegion::isIn(const XYZ& points) const {
    return !_region->isIn(points);
}

double ComplementRegion::getVolume() const {
    return std::numeric_limits<double>::infinity();
}

} // namespace molcpp
