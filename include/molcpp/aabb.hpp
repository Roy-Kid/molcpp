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
        Vec3f d{max[0] - min[0], max[1] - min[1], max[2] - min[2]};
        if (d[0] >= d[1] && d[0] >= d[2]) return 0;
        if (d[1] >= d[0] && d[1] >= d[2]) return 1;
        return 2;
    }

    bool intersects_sphere(const Vec3f& c, float r) const {
        float r2 = r * r;
        float dx = 0.0f;
        if (c[0] < min[0]) dx = min[0] - c[0]; else if (c[0] > max[0]) dx = c[0] - max[0];
        float dy = 0.0f;
        if (c[1] < min[1]) dy = min[1] - c[1]; else if (c[1] > max[1]) dy = c[1] - max[1];
        float dz = 0.0f;
        if (c[2] < min[2]) dz = min[2] - c[2]; else if (c[2] > max[2]) dz = c[2] - max[2];
        return dx*dx + dy*dy + dz*dz <= r2;
    }
};

} // namespace molcpp