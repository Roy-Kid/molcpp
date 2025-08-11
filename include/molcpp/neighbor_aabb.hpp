#pragma once

#include "types.hpp"
#include "box.hpp"
#include "aabb_tree.hpp"

#include <vector>
#include <tuple>
#include <cstddef>
#include <cmath>

#if __has_include(<xsimd/xsimd.hpp>)
  #include <xsimd/xsimd.hpp>
  #define MOLCPP_HAVE_XSIMD 1
#else
  #define MOLCPP_HAVE_XSIMD 0
#endif

namespace molcpp {

class AABBNeighborQuery {
public:
    AABBNeighborQuery(Box box, const std::vector<Vec3f>& points, int leaf_size = 8)
        : m_box(std::move(box)), m_points(points) {
        m_tree.build(m_points, leaf_size);
        split_components();
    }

    // Returns tuple of (indices_i, indices_j, distances)
    std::tuple<std::vector<int>, std::vector<int>, std::vector<float>>
    query(const std::vector<Vec3f>& query_points, float r_max, bool exclude_ii = false) const {
        const float r2_max = r_max * r_max;
        std::vector<int> idx_i;
        std::vector<int> idx_j;
        std::vector<float> distances;
        idx_i.reserve(query_points.size() * 8);
        idx_j.reserve(query_points.size() * 8);
        distances.reserve(query_points.size() * 8);

        const auto shifts = m_box.compute_sphere_image_shifts(r_max);

        // Use thread-local buffers to avoid contention
        #pragma omp parallel if(query_points.size() > 256)
        {
            std::vector<int> tl_i;
            std::vector<int> tl_j;
            std::vector<float> tl_d;
            tl_i.reserve(256);
            tl_j.reserve(256);
            tl_d.reserve(256);

            #pragma omp for schedule(static)
            for (std::ptrdiff_t qi = 0; qi < static_cast<std::ptrdiff_t>(query_points.size()); ++qi) {
                const Vec3f q = query_points[qi];
                for (const Vec3f& shift : shifts) {
                    Vec3f qc{q.x + shift.x, q.y + shift.y, q.z + shift.z};
                    traverse_and_collect(qc, qi, r_max, r2_max, exclude_ii, tl_i, tl_j, tl_d);
                }
            }

            #pragma omp critical
            {
                idx_i.insert(idx_i.end(), tl_i.begin(), tl_i.end());
                idx_j.insert(idx_j.end(), tl_j.begin(), tl_j.end());
                distances.insert(distances.end(), tl_d.begin(), tl_d.end());
            }
        }

        return {std::move(idx_i), std::move(idx_j), std::move(distances)};
    }

private:
    void split_components() {
        const size_t n = m_points.size();
        xs.resize(n); ys.resize(n); zs.resize(n);
        for (size_t i = 0; i < n; ++i) {
            xs[i] = m_points[i].x;
            ys[i] = m_points[i].y;
            zs[i] = m_points[i].z;
        }
    }

    inline void traverse_and_collect(const Vec3f& qc, int qi, float r_max, float r2_max, bool exclude_ii,
                                     std::vector<int>& out_i, std::vector<int>& out_j, std::vector<float>& out_d) const {
        const auto& nodes = m_tree.nodes();
        if (nodes.empty()) return;

        // Manual stack to avoid recursion
        int stack[128];
        int sp = 0;
        stack[sp++] = 0; // root index

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

        const float Lx = m_box.Lx();
        const float Ly = m_box.Ly();
        const float Lz = m_box.Lz();
        const bool px = m_box.periodic_x();
        const bool py = m_box.periodic_y();
        const bool pz = m_box.periodic_z();

#if MOLCPP_HAVE_XSIMD
        using batch = xsimd::batch<float>;
        constexpr std::size_t L = batch::size;

        batch qc_x(qc.x), qc_y(qc.y), qc_z(qc.z);
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
        // Remainder or full scalar loop
        for (; i < end; ++i) {
            int j = idx[i];
            if (exclude_ii && j == qi) continue;
            float dx = xs[j] - qc.x;
            float dy = ys[j] - qc.y;
            float dz = zs[j] - qc.z;
            if (px) dx -= std::round(dx / Lx) * Lx;
            if (py) dy -= std::round(dy / Ly) * Ly;
            if (pz) dz -= std::round(dz / Lz) * Lz;
            float d2 = dx * dx + dy * dy + dz * dz;
            if (d2 <= r2_max) {
                out_i.push_back(qi);
                out_j.push_back(j);
                out_d.push_back(std::sqrt(d2));
            }
        }
    }

    Box m_box;
    std::vector<Vec3f> m_points;
    AABBTree m_tree;
    std::vector<float> xs, ys, zs; // SoA for SIMD
};

} // namespace molcpp