#ifndef MOLCPP_LOCALITY_AABBTREE_HPP
#define MOLCPP_LOCALITY_AABBTREE_HPP

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <xtensor/xarray.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xadapt.hpp>

#include "AABB.hpp"
#include "../types.hpp"

namespace molcpp {
namespace locality {

constexpr unsigned int NODE_CAPACITY = 16;        //!< Maximum number of particles in a node
constexpr unsigned int INVALID_NODE = 0xffffffff; //!< Invalid node index sentinel

//! Node in an AABBTree
/*! Stores data for a node in the AABB tree */
struct CACHE_ALIGN AABBNode {
    //! Default constructor
    AABBNode() {
        left = right = parent = INVALID_NODE;
        num_particles = 0;
        skip = 0;
    }

    AABB aabb;           //!< The box bounding this node's volume
    unsigned int left;   //!< Index of the left child
    unsigned int right;  //!< Index of the right child
    unsigned int parent; //!< Index of the parent node
    unsigned int skip;   //!< Number of array indices to skip to get to the next node in an in order traversal

    std::array<unsigned int, NODE_CAPACITY> particles; //!< Indices of the particles contained in the node
    std::array<unsigned int, NODE_CAPACITY> particle_tags; //!< Corresponding particle tags for particles in node
    unsigned int num_particles; //!< Number of particles contained in the node
};

//! AABB Tree
/*! An AABBTree stores a binary tree of AABBs. A leaf node stores up to NODE_CAPACITY particles by index.
    The bounding box of a leaf node surrounds all the bounding boxes of its contained particles.
    Internal nodes have AABBs that enclose all of their children.
*/
class AABBTree {
public:
    //! Construct an AABBTree
    AABBTree() : m_nodes(nullptr), m_num_nodes(0), m_node_capacity(0), m_root(INVALID_NODE) {}

    //! Destructor
    ~AABBTree() {
        if (m_nodes != nullptr) {
            std::free(m_nodes);
        }
    }

    //! Copy constructor
    AABBTree(const AABBTree& from)
        : m_num_nodes(from.m_num_nodes), m_node_capacity(from.m_node_capacity), 
          m_root(from.m_root), m_mapping(from.m_mapping) {
        if (from.m_nodes != nullptr) {
            // Allocate aligned memory
            m_nodes = static_cast<AABBNode*>(std::aligned_alloc(32, m_node_capacity * sizeof(AABBNode)));
            if (m_nodes == nullptr) {
                throw std::runtime_error("Error allocating AABBTree memory");
            }
            // Copy over data
            std::copy(from.m_nodes, from.m_nodes + m_num_nodes, m_nodes);
        }
    }

    //! Copy assignment
    AABBTree& operator=(const AABBTree& from) {
        if (this == &from) {
            return *this;
        }
        m_num_nodes = from.m_num_nodes;
        m_node_capacity = from.m_node_capacity;
        m_root = from.m_root;
        m_mapping = from.m_mapping;

        if (m_nodes != nullptr) {
            std::free(m_nodes);
        }

        m_nodes = nullptr;

        if (from.m_nodes != nullptr) {
            m_nodes = static_cast<AABBNode*>(std::aligned_alloc(32, m_node_capacity * sizeof(AABBNode)));
            if (m_nodes == nullptr) {
                throw std::runtime_error("Error allocating AABBTree memory");
            }
            std::copy(from.m_nodes, from.m_nodes + m_num_nodes, m_nodes);
        }

        return *this;
    }

    //! Move constructor
    AABBTree(AABBTree&& from) noexcept
        : m_nodes(from.m_nodes), m_num_nodes(from.m_num_nodes), 
          m_node_capacity(from.m_node_capacity), m_root(from.m_root),
          m_mapping(std::move(from.m_mapping)) {
        from.m_nodes = nullptr;
        from.m_num_nodes = 0;
        from.m_node_capacity = 0;
        from.m_root = INVALID_NODE;
    }

    //! Move assignment
    AABBTree& operator=(AABBTree&& from) noexcept {
        if (this == &from) {
            return *this;
        }

        if (m_nodes != nullptr) {
            std::free(m_nodes);
        }

        m_nodes = from.m_nodes;
        m_num_nodes = from.m_num_nodes;
        m_node_capacity = from.m_node_capacity;
        m_root = from.m_root;
        m_mapping = std::move(from.m_mapping);

        from.m_nodes = nullptr;
        from.m_num_nodes = 0;
        from.m_node_capacity = 0;
        from.m_root = INVALID_NODE;

        return *this;
    }

    //! Build the AABBTree from a set of AABBs
    /*! \param aabbs List of AABBs, one per particle
        \param parallelism Use OpenMP parallelization if available
    */
    void buildTree(const std::vector<AABB>& aabbs, bool parallelism = true);

    //! Query the AABBTree for overlaps
    /*! \param aabb Query AABB
        \param result Vector to store indices of overlapping particles
    */
    void query(const AABB& aabb, std::vector<unsigned int>& result) const;

    //! Query the AABBTree for overlaps with a sphere
    /*! \param sphere Query sphere
        \param result Vector to store indices of overlapping particles
    */
    void query(const AABBSphere& sphere, std::vector<unsigned int>& result) const;

    //! Update the AABB for a particle
    /*! \param idx Particle index
        \param aabb New AABB for the particle
    */
    void update(unsigned int idx, const AABB& aabb);

    //! Get the root node index
    unsigned int getRoot() const { return m_root; }

    //! Get the number of nodes
    unsigned int getNumNodes() const { return m_num_nodes; }

    //! Get a node by index
    const AABBNode& getNode(unsigned int idx) const {
        if (idx >= m_num_nodes) {
            throw std::out_of_range("Node index out of range");
        }
        return m_nodes[idx];
    }

protected:
    //! Allocate a new node
    unsigned int allocate() {
        if (m_num_nodes >= m_node_capacity) {
            // Need to grow the array
            unsigned int new_capacity = std::max(16u, m_node_capacity * 2);
            AABBNode* new_nodes = static_cast<AABBNode*>(
                std::aligned_alloc(32, new_capacity * sizeof(AABBNode)));
            if (new_nodes == nullptr) {
                throw std::runtime_error("Error allocating AABBTree memory");
            }

            if (m_nodes != nullptr) {
                std::copy(m_nodes, m_nodes + m_num_nodes, new_nodes);
                std::free(m_nodes);
            }

            m_nodes = new_nodes;
            m_node_capacity = new_capacity;
        }

        unsigned int idx = m_num_nodes++;
        new (&m_nodes[idx]) AABBNode();
        return idx;
    }

    //! Build tree recursively
    unsigned int buildRecursive(std::vector<AABB>& aabbs, 
                                std::vector<unsigned int>& indices,
                                unsigned int start, unsigned int end);

    //! Query tree recursively for AABB overlaps
    void queryRecursive(unsigned int node_idx, const AABB& aabb,
                       std::vector<unsigned int>& result) const;

    //! Query tree recursively for sphere overlaps
    void queryRecursive(unsigned int node_idx, const AABBSphere& sphere,
                       std::vector<unsigned int>& result) const;

    //! Update AABBs recursively up the tree
    void updateRecursive(unsigned int node_idx);

    AABBNode* m_nodes;                           //!< Array of nodes
    unsigned int m_num_nodes;                    //!< Number of nodes in use
    unsigned int m_node_capacity;                //!< Capacity of the node array
    unsigned int m_root;                         //!< Root node index
    std::vector<unsigned int> m_mapping;         //!< Mapping from particle index to leaf node
};

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_AABBTREE_HPP