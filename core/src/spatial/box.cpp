#include "molcpp/spatial/box.hpp"
#include "molcpp/spatial/boundary.hpp"
#include "molcpp/spatial/region.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor-blas/xlinalg.hpp>
#include <memory>

namespace molcpp {

using XYZ = xt::xarray<float>;

Box::Box(const Mat3<float>& matrix, const Vec3<float>& origin, const Vec3<bool>& pbc)
    : _pbc(pbc), _region(matrix, origin) {
    _boundary = createBoundary(pbc);
}

// Copy constructor
Box::Box(const Box& other)
    : _pbc(other._pbc), _region(other._region)
{
    if (other._boundary) {
        if (dynamic_cast<OpenBoundary*>(other._boundary.get())) {
            _boundary = std::make_unique<OpenBoundary>();
        } else if (dynamic_cast<PeriodicBoundary*>(other._boundary.get())) {
            auto* pb = dynamic_cast<PeriodicBoundary*>(other._boundary.get());
            _boundary = std::make_unique<PeriodicBoundary>(pb->getPBC(0), pb->getPBC(1), pb->getPBC(2));
        }
    }
}

// Copy assignment operator
Box& Box::operator=(const Box& other)
{
    if (this != &other) {
        _pbc = other._pbc;
        _region = other._region;
        
        if (other._boundary) {
            if (dynamic_cast<OpenBoundary*>(other._boundary.get())) {
                _boundary = std::make_unique<OpenBoundary>();
            } else if (dynamic_cast<PeriodicBoundary*>(other._boundary.get())) {
                auto* pb = dynamic_cast<PeriodicBoundary*>(other._boundary.get());
                _boundary = std::make_unique<PeriodicBoundary>(pb->getPBC(0), pb->getPBC(1), pb->getPBC(2));
            }
        }
    }
    return *this;
}

// Move constructor
Box::Box(Box&& other) noexcept
    : _pbc(std::move(other._pbc)), _region(std::move(other._region)), _boundary(std::move(other._boundary))
{
}

// Move assignment operator
Box& Box::operator=(Box&& other) noexcept
{
    if (this != &other) {
        _pbc = std::move(other._pbc);
        _region = std::move(other._region);
        _boundary = std::move(other._boundary);
    }
    return *this;
}

// Destructor
Box::~Box()
{
}

Box Box::cube(float length, const Vec3<float>& origin, const Vec3<bool>& pbc) {
    std::vector<size_t> shape = {3, 3};
    Mat3<float> matrix = xt::zeros<float>(shape);
    matrix(0, 0) = length;
    matrix(1, 1) = length;
    matrix(2, 2) = length;
    return Box(matrix, origin, pbc);
}

Box Box::orthorhombic(const Vec3<float>& lengths, const Vec3<float>& origin, const Vec3<bool>& pbc) {
    std::vector<size_t> shape = {3, 3};
    Mat3<float> matrix = xt::zeros<float>(shape);
    matrix(0, 0) = lengths(0);
    matrix(1, 1) = lengths(1);
    matrix(2, 2) = lengths(2);
    return Box(matrix, origin, pbc);
}

XYZ Box::toFrac(const XYZ& cart) const {
    // Get matrix and origin from the region
    const auto& matrix = _region.getMatrix();
    const auto& origin = _region.getOrigin();
    
    // Calculate inverse matrix
    XYZ inv_matrix = xt::linalg::inv(matrix);
    
    // frac = H^{-1} @ (cart - origin)
    return xt::linalg::dot(cart - origin, inv_matrix);
}

XYZ Box::toCart(const XYZ& frac) const {
    // Get matrix and origin from the region
    const auto& matrix = _region.getMatrix();
    const auto& origin = _region.getOrigin();
    
    // cart = origin + H @ frac
    return origin + xt::linalg::dot(frac, matrix);
}

xt::xarray<bool> Box::isIn(const XYZ& points) const {
    // Delegate to the geometric region
    return _region.isIn(points);
}

double Box::getVolume() const {
    // Delegate to the geometric region
    return _region.getVolume();
}

XYZ Box::wrap(const XYZ& points) const {
    // Delegate to boundary strategy
    return _boundary->wrap(*this, points);
}

XYZ Box::delta(const XYZ& a, const XYZ& b, bool minimumImage) const {
    // Delegate to boundary strategy
    return _boundary->delta(*this, a, b, minimumImage);
}

// Additional methods for backward compatibility
Vec3<float> Box::getNearestPlaneDistance() const {
    // For a box, the nearest plane distance is half the box length in each dimension
    // This represents the distance from the center to the nearest face
    Mat3<float> const& mat = matrix();
    Vec3<float> result;
    
    // For orthogonal boxes, this is simply half the diagonal elements
    // For triclinic boxes, we need to consider the actual box dimensions
    result[0] = std::abs(mat(0, 0)) / 2.0f;
    result[1] = std::abs(mat(1, 1)) / 2.0f;
    result[2] = std::abs(mat(2, 2)) / 2.0f;
    
    return result;
}

Vec3<float> Box::getLatticeVector(int index) const {
    Mat3<float> const& mat = matrix();
    Vec3<float> result;
    result[0] = mat(index, 0);
    result[1] = mat(index, 1);
    result[2] = mat(index, 2);
    return result;
}

} // namespace molcpp