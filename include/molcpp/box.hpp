#pragma once

#include "types.hpp"
#include <array>
#include <vector>

namespace molcpp {

class Box {
public:
    Box(float Lx, float Ly, float Lz, bool periodic_x = true, bool periodic_y = true, bool periodic_z = true)
        : m_Lx(Lx), m_Ly(Ly), m_Lz(Lz) {
        m_periodic[0] = periodic_x;
        m_periodic[1] = periodic_y;
        m_periodic[2] = periodic_z;
    }

    float Lx() const { return m_Lx; }
    float Ly() const { return m_Ly; }
    float Lz() const { return m_Lz; }

    bool periodic_x() const { return m_periodic[0]; }
    bool periodic_y() const { return m_periodic[1]; }
    bool periodic_z() const { return m_periodic[2]; }

    Vec3f wrap(const Vec3f& p) const {
        Vec3f r = p;
        if (m_periodic[0]) r[0] = wrap_component(r[0], m_Lx);
        if (m_periodic[1]) r[1] = wrap_component(r[1], m_Ly);
        if (m_periodic[2]) r[2] = wrap_component(r[2], m_Lz);
        return r;
    }

    Vec3f minimum_image(const Vec3f& dr) const {
        Vec3f out = dr;
        if (m_periodic[0]) out[0] = minimum_image_component(out[0], m_Lx);
        if (m_periodic[1]) out[1] = minimum_image_component(out[1], m_Ly);
        if (m_periodic[2]) out[2] = minimum_image_component(out[2], m_Lz);
        return out;
    }

    // Returns the 27 image shift vectors needed for sphere queries up to radius r_max
    std::vector<Vec3f> compute_sphere_image_shifts(float /*r_max*/) const {
        std::vector<Vec3f> shifts;
        shifts.emplace_back(Vec3f{0.0f, 0.0f, 0.0f});
        const int nx = m_periodic[0] ? 3 : 1; // -1, 0, +1
        const int ny = m_periodic[1] ? 3 : 1;
        const int nz = m_periodic[2] ? 3 : 1;
        for (int ix = 0; ix < nx; ++ix) {
            for (int iy = 0; iy < ny; ++iy) {
                for (int iz = 0; iz < nz; ++iz) {
                    int sx = ix - 1;
                    int sy = iy - 1;
                    int sz = iz - 1;
                    if (sx == 0 && sy == 0 && sz == 0) continue;
                    Vec3f shift{sx * m_Lx, sy * m_Ly, sz * m_Lz};
                    shifts.emplace_back(shift);
                }
            }
        }
        return shifts;
    }

private:
    static inline float wrap_component(float value, float L) {
        float n = std::floor(value / L + 0.5f);
        return value - n * L;
    }

    static inline float minimum_image_component(float dx, float L) {
        float n = std::round(dx / L);
        return dx - n * L;
    }

    float m_Lx;
    float m_Ly;
    float m_Lz;
    std::array<bool, 3> m_periodic{};
};

} // namespace molcpp