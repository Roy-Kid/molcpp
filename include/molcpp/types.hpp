#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <algorithm>

#include <xtensor/xtensor.hpp>

namespace molcpp {

using Vec3f = xt::xtensor_fixed<float, xt::xshape<3>>;

inline Vec3f make_vec3f(float x, float y, float z) {
    return Vec3f{ x, y, z };
}

inline Vec3f vadd(const Vec3f& a, const Vec3f& b) { return Vec3f{a[0] + b[0], a[1] + b[1], a[2] + b[2]}; }
inline Vec3f vsub(const Vec3f& a, const Vec3f& b) { return Vec3f{a[0] - b[0], a[1] - b[1], a[2] - b[2]}; }
inline Vec3f vscale(const Vec3f& a, float s) { return Vec3f{a[0] * s, a[1] * s, a[2] * s}; }

inline float dot(const Vec3f& a, const Vec3f& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
inline float norm2(const Vec3f& a) { return dot(a, a); }
inline float norm(const Vec3f& a) { return std::sqrt(norm2(a)); }

inline Vec3f vmin(const Vec3f& a, const Vec3f& b) {
    return Vec3f{std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2])};
}
inline Vec3f vmax(const Vec3f& a, const Vec3f& b) {
    return Vec3f{std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2])};
}

} // namespace molcpp