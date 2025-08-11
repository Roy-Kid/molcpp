#pragma once

#include "types.hpp"
#include "aabb.hpp"
#include <vector>
#include <numeric>
#include <algorithm>

namespace molcpp {

struct AABBNode {
    AABB bbox;
    int left = -1;
    int right = -1;
    int start = 0;
    int count = 0;
    bool is_leaf() const { return count > 0; }
};

class AABBTree {
public:
    AABBTree() = default;

    void build(const std::vector<Vec3f>& points, int leaf_size = 8) {
        m_points = &points;
        m_leaf_size = std::max(1, leaf_size);
        m_indices.resize(points.size());
        std::iota(m_indices.begin(), m_indices.end(), 0);
        m_nodes.clear();
        if (points.empty()) return;
        build_node(0, static_cast<int>(points.size()));
    }

    const std::vector<AABBNode>& nodes() const { return m_nodes; }
    const std::vector<int>& indices() const { return m_indices; }

private:
    int build_node(int start, int end) {
        AABBNode node;
        node.start = start;
        node.count = end - start;

        // Compute bounding box
        for (int i = start; i < end; ++i) {
            const Vec3f& p = (*m_points)[m_indices[i]];
            node.bbox.expand(p);
        }

        int node_index = static_cast<int>(m_nodes.size());
        m_nodes.push_back(node);

        if (node.count <= m_leaf_size) {
            m_nodes[node_index].left = -1;
            m_nodes[node_index].right = -1;
            return node_index;
        }

        int axis = node.bbox.longest_axis();
        int mid = (start + end) / 2;

        auto comparator = [&](int lhs, int rhs) {
            const Vec3f& a = (*m_points)[lhs];
            const Vec3f& b = (*m_points)[rhs];
            if (axis == 0) return a[0] < b[0];
            if (axis == 1) return a[1] < b[1];
            return a[2] < b[2];
        };

        std::nth_element(m_indices.begin() + start, m_indices.begin() + mid, m_indices.begin() + end,
                         [&](int i1, int i2) { return comparator(i1, i2); });

        int left = build_node(start, mid);
        int right = build_node(mid, end);

        m_nodes[node_index].left = left;
        m_nodes[node_index].right = right;
        m_nodes[node_index].count = 0; // internal

        m_nodes[node_index].bbox = AABB{};
        m_nodes[node_index].bbox.expand(m_nodes[left].bbox);
        m_nodes[node_index].bbox.expand(m_nodes[right].bbox);

        return node_index;
    }

    const std::vector<Vec3f>* m_points{nullptr};
    std::vector<int> m_indices;
    std::vector<AABBNode> m_nodes;
    int m_leaf_size{8};
};

} // namespace molcpp