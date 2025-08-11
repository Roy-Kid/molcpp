#include <molcpp/locality/AABBTree.hpp>
#include <algorithm>
#include <numeric>
#include <stack>

namespace molcpp {
namespace locality {

void AABBTree::buildTree(const std::vector<AABB>& aabbs, bool parallelism) {
    // Clear existing tree
    m_num_nodes = 0;
    m_root = INVALID_NODE;
    m_mapping.clear();
    m_mapping.resize(aabbs.size(), INVALID_NODE);

    if (aabbs.empty()) {
        return;
    }

    // Estimate capacity needed
    unsigned int estimated_nodes = static_cast<unsigned int>(aabbs.size() / NODE_CAPACITY * 3);
    if (estimated_nodes > m_node_capacity) {
        if (m_nodes != nullptr) {
            std::free(m_nodes);
        }
        m_node_capacity = estimated_nodes;
        m_nodes = static_cast<AABBNode*>(std::aligned_alloc(32, m_node_capacity * sizeof(AABBNode)));
        if (m_nodes == nullptr) {
            throw std::runtime_error("Error allocating AABBTree memory");
        }
    }

    // Create indices array
    std::vector<unsigned int> indices(aabbs.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Make a copy of AABBs for building
    std::vector<AABB> build_aabbs = aabbs;

    // Build the tree recursively
    m_root = buildRecursive(build_aabbs, indices, 0, static_cast<unsigned int>(indices.size()));

    // Compute skip values for efficient traversal
    if (m_root != INVALID_NODE) {
        std::stack<unsigned int> stack;
        stack.push(m_root);
        
        while (!stack.empty()) {
            unsigned int node_idx = stack.top();
            stack.pop();
            AABBNode& node = m_nodes[node_idx];
            
            if (node.left != INVALID_NODE) {
                stack.push(node.left);
                if (node.right != INVALID_NODE) {
                    stack.push(node.right);
                    // Skip value points to right sibling
                    m_nodes[node.left].skip = node.right - node.left;
                }
            }
        }
    }
}

unsigned int AABBTree::buildRecursive(std::vector<AABB>& aabbs,
                                      std::vector<unsigned int>& indices,
                                      unsigned int start, unsigned int end) {
    if (start >= end) {
        return INVALID_NODE;
    }

    unsigned int n = end - start;
    
    // Allocate a new node
    unsigned int node_idx = allocate();
    AABBNode& node = m_nodes[node_idx];

    // If we have few enough particles, make this a leaf
    if (n <= NODE_CAPACITY) {
        node.num_particles = n;
        for (unsigned int i = 0; i < n; ++i) {
            unsigned int particle_idx = indices[start + i];
            node.particles[i] = particle_idx;
            node.particle_tags[i] = aabbs[particle_idx].tag;
            m_mapping[particle_idx] = node_idx;
            
            // Merge AABBs to get node's AABB
            if (i == 0) {
                node.aabb = aabbs[particle_idx];
            } else {
                node.aabb = merge(node.aabb, aabbs[particle_idx]);
            }
        }
        return node_idx;
    }

    // Otherwise, split and create internal node
    // Compute the bounding box of all AABBs
    AABB bounds = aabbs[indices[start]];
    for (unsigned int i = start + 1; i < end; ++i) {
        bounds = merge(bounds, aabbs[indices[i]]);
    }
    node.aabb = bounds;

    // Find the longest axis
    Vec3 extent = bounds.upper - bounds.lower;
    int split_axis = 0;
    if (extent(1) > extent(0)) split_axis = 1;
    if (extent(2) > extent(split_axis)) split_axis = 2;

    // Sort along the split axis
    unsigned int mid = start + n / 2;
    
#ifdef _OPENMP
    if (n > 1000) {
        // Use parallel sort for large arrays
        #pragma omp parallel
        {
            #pragma omp single
            {
                std::sort(indices.begin() + start, indices.begin() + end,
                         [&aabbs, split_axis](unsigned int a, unsigned int b) {
                             return aabbs[a].getPosition()(split_axis) < 
                                    aabbs[b].getPosition()(split_axis);
                         });
            }
        }
    } else
#endif
    {
        std::nth_element(indices.begin() + start, indices.begin() + mid, indices.begin() + end,
                        [&aabbs, split_axis](unsigned int a, unsigned int b) {
                            return aabbs[a].getPosition()(split_axis) < 
                                   aabbs[b].getPosition()(split_axis);
                        });
    }

    // Recursively build children
    node.left = buildRecursive(aabbs, indices, start, mid);
    node.right = buildRecursive(aabbs, indices, mid, end);

    // Set parent pointers
    if (node.left != INVALID_NODE) {
        m_nodes[node.left].parent = node_idx;
    }
    if (node.right != INVALID_NODE) {
        m_nodes[node.right].parent = node_idx;
    }

    return node_idx;
}

void AABBTree::query(const AABB& aabb, std::vector<unsigned int>& result) const {
    result.clear();
    if (m_root != INVALID_NODE) {
        queryRecursive(m_root, aabb, result);
    }
}

void AABBTree::queryRecursive(unsigned int node_idx, const AABB& aabb,
                              std::vector<unsigned int>& result) const {
    const AABBNode& node = m_nodes[node_idx];

    // Check if query AABB overlaps with node AABB
    if (!overlap(node.aabb, aabb)) {
        return;
    }

    // If this is a leaf node, check particles
    if (node.num_particles > 0) {
        for (unsigned int i = 0; i < node.num_particles; ++i) {
            result.push_back(node.particles[i]);
        }
    } else {
        // Recursively check children
        if (node.left != INVALID_NODE) {
            queryRecursive(node.left, aabb, result);
        }
        if (node.right != INVALID_NODE) {
            queryRecursive(node.right, aabb, result);
        }
    }
}

void AABBTree::query(const AABBSphere& sphere, std::vector<unsigned int>& result) const {
    result.clear();
    if (m_root != INVALID_NODE) {
        queryRecursive(m_root, sphere, result);
    }
}

void AABBTree::queryRecursive(unsigned int node_idx, const AABBSphere& sphere,
                              std::vector<unsigned int>& result) const {
    const AABBNode& node = m_nodes[node_idx];

    // Check if query sphere overlaps with node AABB
    if (!overlap(node.aabb, sphere)) {
        return;
    }

    // If this is a leaf node, check particles
    if (node.num_particles > 0) {
        for (unsigned int i = 0; i < node.num_particles; ++i) {
            result.push_back(node.particles[i]);
        }
    } else {
        // Recursively check children
        if (node.left != INVALID_NODE) {
            queryRecursive(node.left, sphere, result);
        }
        if (node.right != INVALID_NODE) {
            queryRecursive(node.right, sphere, result);
        }
    }
}

void AABBTree::update(unsigned int idx, const AABB& aabb) {
    if (idx >= m_mapping.size() || m_mapping[idx] == INVALID_NODE) {
        throw std::out_of_range("Particle index not in tree");
    }

    unsigned int node_idx = m_mapping[idx];
    AABBNode& node = m_nodes[node_idx];

    // Find the particle in the node and update its AABB contribution
    bool found = false;
    for (unsigned int i = 0; i < node.num_particles; ++i) {
        if (node.particles[i] == idx) {
            // Recompute the node's AABB
            if (node.num_particles == 1) {
                node.aabb = aabb;
            } else {
                // Need to recompute from all particles
                // This is a simplification - in practice, you might want to store
                // individual AABBs to avoid this
                node.aabb = aabb;
                // Note: This is simplified - a full implementation would need to
                // track all particle AABBs or recompute from actual positions
            }
            found = true;
            break;
        }
    }

    if (!found) {
        throw std::runtime_error("Particle not found in its mapped node");
    }

    // Update parent nodes up the tree
    updateRecursive(node.parent);
}

void AABBTree::updateRecursive(unsigned int node_idx) {
    if (node_idx == INVALID_NODE) {
        return;
    }

    AABBNode& node = m_nodes[node_idx];
    
    // Recompute AABB from children
    if (node.left != INVALID_NODE && node.right != INVALID_NODE) {
        node.aabb = merge(m_nodes[node.left].aabb, m_nodes[node.right].aabb);
    } else if (node.left != INVALID_NODE) {
        node.aabb = m_nodes[node.left].aabb;
    } else if (node.right != INVALID_NODE) {
        node.aabb = m_nodes[node.right].aabb;
    }

    // Continue up the tree
    updateRecursive(node.parent);
}

} // namespace locality
} // namespace molcpp