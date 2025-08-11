#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <algorithm>

namespace molcpp {

struct Vec3f {
    float x;
    float y;
    float z;

    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

inline Vec3f operator+(const Vec3f& a, const Vec3f& b) { return Vec3f{a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3f operator-(const Vec3f& a, const Vec3f& b) { return Vec3f{a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3f operator*(const Vec3f& a, float s) { return Vec3f{a.x * s, a.y * s, a.z * s}; }
inline Vec3f operator*(float s, const Vec3f& a) { return a * s; }

inline float dot(const Vec3f& a, const Vec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float norm2(const Vec3f& a) { return dot(a, a); }
inline float norm(const Vec3f& a) { return std::sqrt(norm2(a)); }

inline Vec3f vmin(const Vec3f& a, const Vec3f& b) {
    return Vec3f{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}
inline Vec3f vmax(const Vec3f& a, const Vec3f& b) {
    return Vec3f{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

} // namespace molcpp