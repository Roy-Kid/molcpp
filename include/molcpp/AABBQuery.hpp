#pragma once

#include "nl_types.hpp"
#include "aabb_tree.hpp"
#include "NeighborList.hpp"

#include <vector>
#include <tuple>
#include <cstddef>
#include <cmath>

#include <xtensor/xtensor.hpp>

#if __has_include(<xsimd/xsimd.hpp>)
  #include <xsimd/xsimd.hpp>
  #define MOLCPP_HAVE_XSIMD 1
#else
  #define MOLCPP_HAVE_XSIMD 0
#endif

namespace molcpp {

class AABBQuery {
public:
    template <class BoxLike>
    AABBQuery(const xt::xtensor<float, 2>& points, const BoxLike& box_like, int leaf_size = 8)
        : m_Lx(box_like.Lx()), m_Ly(box_like.Ly()), m_Lz(box_like.Lz()),
          m_px(box_like.periodic_x()), m_py(box_like.periodic_y()), m_pz(box_like.periodic_z()) {
        init_points(points, leaf_size);
    }

    auto query(const xt::xtensor<float, 2>& query_points, float r_max, bool exclude_ii = false) const
        -> std::tuple<xt::xtensor<int, 1>, xt::xtensor<int, 1>, xt::xtensor<float, 1>>
    {
        if (query_points.dimension() != 2 || query_points.shape(1) != 3) {
            throw std::runtime_error("query_points must have shape (M, 3)");
        }

        const float r2_max = r_max * r_max;
        std::vector<int> idx_i;
        std::vector<int> idx_j;
        std::vector<float> distances;
        idx_i.reserve(query_points.shape(0) * 8);
        idx_j.reserve(query_points.shape(0) * 8);
        distances.reserve(query_points.shape(0) * 8);

        const auto shifts = compute_sphere_image_shifts(r_max);

        #pragma omp parallel if(query_points.shape(0) > 256)
        {
            std::vector<int> tl_i;
            std::vector<int> tl_j;
            std::vector<float> tl_d;
            tl_i.reserve(256);
            tl_j.reserve(256);
            tl_d.reserve(256);

            #pragma omp for schedule(static)
            for (std::ptrdiff_t qi = 0; qi < static_cast<std::ptrdiff_t>(query_points.shape(0)); ++qi) {
                Vec3f q{query_points(qi, 0), query_points(qi, 1), query_points(qi, 2)};
                for (const Vec3f& shift : shifts) {
                    Vec3f qc{q[0] + shift[0], q[1] + shift[1], q[2] + shift[2]};
                    traverse_and_collect(qc, static_cast<int>(qi), r_max, r2_max, exclude_ii, tl_i, tl_j, tl_d);
                }
            }

            #pragma omp critical
            {
                idx_i.insert(idx_i.end(), tl_i.begin(), tl_i.end());
                idx_j.insert(idx_j.end(), tl_j.begin(), tl_j.end());
                distances.insert(distances.end(), tl_d.begin(), tl_d.end());
            }
        }

        xt::xtensor<int, 1> ai = xt::zeros<int>({idx_i.size()});
        xt::xtensor<int, 1> aj = xt::zeros<int>({idx_j.size()});
        xt::xtensor<float, 1> ad = xt::zeros<float>({distances.size()});
        for (std::size_t k = 0; k < idx_i.size(); ++k) ai(k) = idx_i[k];
        for (std::size_t k = 0; k < idx_j.size(); ++k) aj(k) = idx_j[k];
        for (std::size_t k = 0; k < distances.size(); ++k) ad(k) = distances[k];
        return {std::move(ai), std::move(aj), std::move(ad)};
    }

    NeighborList query_list(const xt::xtensor<float, 2>& query_points, float r_max, bool exclude_ii = false) const {
        auto [ai, aj, ad] = query(query_points, r_max, exclude_ii);
        return NeighborList(std::move(ai), std::move(aj), std::move(ad));
    }

private:
    void init_points(const xt::xtensor<float, 2>& points, int leaf_size) {
        const auto& sh = points.shape();
        if (sh.size() != 2 || sh[1] != 3) {
            throw std::runtime_error("points must have shape (N, 3)");
        }
        const std::size_t n = sh[0];
        m_points.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            m_points[i] = Vec3f{points(i, 0), points(i, 1), points(i, 2)};
        }
        m_tree.build(m_points, leaf_size);
        split_components();
    }

    std::vector<Vec3f> compute_sphere_image_shifts(float /*r_max*/) const {
        std::vector<Vec3f> shifts;
        shifts.emplace_back(Vec3f{0.0f, 0.0f, 0.0f});
        const int nx = m_px ? 3 : 1;
        const int ny = m_py ? 3 : 1;
        const int nz = m_pz ? 3 : 1;
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

    void split_components() {
        const size_t n = m_points.size();
        xs.resize(n); ys.resize(n); zs.resize(n);
        for (size_t i = 0; i < n; ++i) {
            xs[i] = m_points[i][0];
            ys[i] = m_points[i][1];
            zs[i] = m_points[i][2];
        }
    }

    inline void traverse_and_collect(const Vec3f& qc, int qi, float r_max, float r2_max, bool exclude_ii,
                                     std::vector<int>& out_i, std::vector<int>& out_j, std::vector<float>& out_d) const {
        const auto& nodes = m_tree.nodes();
        if (nodes.empty()) return;

        int stack[128];
        int sp = 0;
        stack[sp++] = 0;

        while (sp > 0) {
            int node_idx = stack[--sp];
            const AABBNode& node = nodes[node_idx];

            if (!node.bbox.intersects_sphere(qc, r_max)) continue;

            if (node.is_leaf()) {
                leaf_collect(node, qc, qi, r2_max, exclude_ii, out_i, out_j, out_d);
            } else {
                if (node.left >= 0) stack[sp++] = node.left;
                if (node.right >= 0) stack[sp++] = node.right;
            }
        }
    }

    inline void leaf_collect(const AABBNode& node, const Vec3f& qc, int qi, float r2_max, bool exclude_ii,
                             std::vector<int>& out_i, std::vector<int>& out_j, std::vector<float>& out_d) const {
        const std::vector<int>& idx = m_tree.indices();
        int start = node.start;
        int end = node.start + node.count;

#if MOLCPP_HAVE_XSIMD
        const float Lx = m_Lx;
        const float Ly = m_Ly;
        const float Lz = m_Lz;
        const bool px = m_px;
        const bool py = m_py;
        const bool pz = m_pz;

        using batch = xsimd::batch<float>;
        constexpr std::size_t L = batch::size;

        batch qc_x(qc[0]), qc_y(qc[1]), qc_z(qc[2]);
        batch Lx_b(Lx), Ly_b(Ly), Lz_b(Lz);

        int i = start;
        for (; i + static_cast<int>(L) <= end; i += static_cast<int>(L)) {
            alignas(XSIMD_DEFAULT_ALIGNMENT) float bx[L];
            alignas(XSIMD_DEFAULT_ALIGNMENT) float by[L];
            alignas(XSIMD_DEFAULT_ALIGNMENT) float bz[L];

            for (std::size_t k = 0; k < L; ++k) {
                int j = idx[i + static_cast<int>(k)];
                bx[k] = xs[j];
                by[k] = ys[j];
                bz[k] = zs[j];
            }

            batch px_b = batch::load_aligned(bx);
            batch py_b = batch::load_aligned(by);
            batch pz_b = batch::load_aligned(bz);

            batch dx = px_b - qc_x;
            batch dy = py_b - qc_y;
            batch dz = pz_b - qc_z;

            if (px) {
                batch nx = xsimd::nearbyint(dx / Lx_b);
                dx = dx - nx * Lx_b;
            }
            if (py) {
                batch ny = xsimd::nearbyint(dy / Ly_b);
                dy = dy - ny * Ly_b;
            }
            if (pz) {
                batch nz = xsimd::nearbyint(dz / Lz_b);
                dz = dz - nz * Lz_b;
            }

            batch d2 = dx * dx + dy * dy + dz * dz;
            alignas(XSIMD_DEFAULT_ALIGNMENT) float d2_arr[L];
            d2.store_aligned(d2_arr);

            for (std::size_t k = 0; k < L; ++k) {
                int j = idx[i + static_cast<int>(k)];
                if (exclude_ii && j == qi) continue;
                float d2v = d2_arr[k];
                if (d2v <= r2_max) {
                    out_i.push_back(qi);
                    out_j.push_back(j);
                    out_d.push_back(std::sqrt(d2v));
                }
            }
        }
#else
        int i = start;
#endif
        for (; i < end; ++i) {
            int j = idx[i];
            if (exclude_ii && j == qi) continue;
            float dx = xs[j] - qc[0];
            float dy = ys[j] - qc[1];
            float dz = zs[j] - qc[2];
            if (m_px) dx -= std::round(dx / m_Lx) * m_Lx;
            if (m_py) dy -= std::round(dy / m_Ly) * m_Ly;
            if (m_pz) dz -= std::round(dz / m_Lz) * m_Lz;
            float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 <= r2_max) {
                out_i.push_back(qi);
                out_j.push_back(j);
                out_d.push_back(std::sqrt(d2));
            }
        }
    }

    // box
    float m_Lx{0}, m_Ly{0}, m_Lz{0};
    bool m_px{true}, m_py{true}, m_pz{true};

    // points
    std::vector<Vec3f> m_points;
    AABBTree m_tree;
    std::vector<float> xs, ys, zs;
};

} // namespace molcpp