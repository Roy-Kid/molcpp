#pragma once

#include "types.hpp"
#include <limits>

namespace molcpp {

struct AABB {
    Vec3f min;
    Vec3f max;

    AABB()
        : min{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()},
          max{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()} {}

    void expand(const Vec3f& p) {
        min = vmin(min, p);
        max = vmax(max, p);
    }

    void expand(const AABB& other) {
        min = vmin(min, other.min);
        max = vmax(max, other.max);
    }

    int longest_axis() const {
        Vec3f d{max.x - min.x, max.y - min.y, max.z - min.z};
        if (d.x >= d.y && d.x >= d.z) return 0;
        if (d.y >= d.x && d.y >= d.z) return 1;
        return 2;
    }

    bool intersects_sphere(const Vec3f& c, float r) const {
        float r2 = r * r;
        // Clamp point to box
        float dx = 0.0f;
        if (c.x < min.x) dx = min.x - c.x; else if (c.x > max.x) dx = c.x - max.x;
        float dy = 0.0f;
        if (c.y < min.y) dy = min.y - c.y; else if (c.y > max.y) dy = c.y - max.y;
        float dz = 0.0f;
        if (c.z < min.z) dz = min.z - c.z; else if (c.z > max.z) dz = c.z - max.z;
        return dx*dx + dy*dy + dz*dz <= r2;
    }
};

} // namespace molcpp