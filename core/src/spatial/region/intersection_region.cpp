#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <stdexcept>

namespace molcpp {

using XYZ = xt::xarray<float>;

IntersectionRegion::IntersectionRegion(std::vector<std::shared_ptr<Region>> regions)
    : _regions(std::move(regions)) {
    if (_regions.empty()) {
        throw std::invalid_argument("IntersectionRegion requires at least one region");
    }
}

IntersectionRegion::IntersectionRegion(std::shared_ptr<Region> region1, std::shared_ptr<Region> region2) {
    _regions.push_back(std::move(region1));
    _regions.push_back(std::move(region2));
}

xt::xarray<bool> IntersectionRegion::isIn(const XYZ& points) const {
    if (_regions.empty()) {
        return xt::xarray<bool>();
    }
    
    auto result = _regions[0]->isIn(points);
    for (size_t i = 1; i < _regions.size(); ++i) {
        result = result && _regions[i]->isIn(points);
    }
    return result;
}

double IntersectionRegion::getVolume() const {
    // Volume calculation for intersection is complex, return NaN
    return std::numeric_limits<double>::quiet_NaN();
}

} // namespace molcpp
