// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef AABBQUERY_H
#define AABBQUERY_H

#include <algorithm>
#include <cmath>
#include <map>
#include <math.h> // NOLINT(modernize-deprecated-headers): Use std::numbers when c++20 is default.
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "molcpp/locality/AABB.hpp"
#include "molcpp/locality/AABBTree.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/locality/NeighborBond.hpp"
#include "molcpp/locality/NeighborQuery.hpp"
#include "molcpp/types.hpp"

/*! \file AABBQuery.h
 *  \brief Build an AABB tree from points and query it for neighbors.
 * A bounding volume hierarchy (BVH) tree is a binary search tree. It is
 * constructed from axis-aligned bounding boxes (AABBs). The AABB for a node in
 * the tree encloses all child AABBs. A leaf AABB holds multiple particles. The
 * tree is constructed in a balanced way using a heuristic to minimize AABB
 * volume. We build one tree per particle type, and use point AABBs for the
 * particles. The neighbor list is built by traversing down the tree with an
 * AABB that encloses the pairwise cutoff for the particle. Periodic boundaries
 * are treated by translating the query AABB by all possible image vectors,
 * many of which are trivially rejected for not intersecting the root node.
 */

namespace molcpp { namespace locality {

class AABBQuery : public NeighborQuery
{
public:
    //! Constructs the compute
    AABBQuery();

    //! Constructor that builds initial neighborlist by calling build method
    AABBQuery(const Box& box, const XYZ& points);

    //! Destructor
    ~AABBQuery() override;

    //! Implementation of per-particle query for AABBQuery (see NeighborQuery.h for documentation).
    /*! \param query_point The point to find neighbors for.
     *  \param n_query_points The number of query points.
     *  \param qargs The query arguments that should be used to find neighbors.
     */
    std::shared_ptr<NeighborQueryPerPointIterator>
    querySingle(const Vec3<float> query_point, unsigned int query_point_idx, QueryArgs args) const override;

    //! Build Neighborlist
    void build(const Box& box, const XYZ& points);

    //! Add points to current Neighborlist
    void update(const XYZ& points);

    //! Get the AABB tree for iterator access
    const AABBTree& getAABBTree() const { return _aabb_tree; }

protected:
    //! Validate the combination of specified arguments.
    /*! Add to parent function to account for the various arguments
     *  specifically required for AABBQuery nearest neighbor queries.
     */
    void validateQueryArgs(QueryArgs& args) const override
    {
        NeighborQuery::validateQueryArgs(args);
        if (args.mode == QueryType::nearest)
        {
            if (args.scale == DEFAULT_SCALE)
            {
                args.scale = float(1.1);
            }
            else if (args.scale <= float(1.0))
            {
                throw std::runtime_error("The scale query argument must be greater than 1.");
            }

            if (args.r_guess == DEFAULT_R_GUESS)
            {
                // By default, we assume a homogeneous system density and use
                // that to estimate the distance we need to query. This
                // calculation assumes a constant density of N/V, where N is
                // the number of particles and V is the box volume, and it
                // calculates the radius of a sphere that will contain the
                // desired number of neighbors.

                float const r_guess = std::cbrtf(
                    (float(3.0) * static_cast<float>(args.num_neighbors) * _box.getVolume())
                    / (float(4.0) * static_cast<float>(M_PI) * static_cast<float>(getNPoints())));

                // The upper bound is set by the minimum nearest plane distances.
                Vec3<float> const nearest_plane_distance = _box.getNearestPlaneDistance();
                float min_plane_distance = std::min({nearest_plane_distance[0], nearest_plane_distance[1], nearest_plane_distance[2]});

                args.r_guess = std::min(r_guess, min_plane_distance / float(2.0));
            }
            if (args.r_guess > args.r_max)
            {
                // No need to search past the requested bounds even if requested.
                args.r_guess = args.r_max;
            }
        }
    }

private:

    //! AABB tree for efficient neighbor finding
    AABBTree _aabb_tree;
    
    //! Vector of AABBs for each point
    std::vector<AABB> _aabbs;

    //! Driver for tree configuration
    void setupTree(unsigned int N);

    //! Maps particles by local id to their id within their type trees
    void mapParticlesByType();

    //! Driver to build AABB trees
    void buildTree(const XYZ& points);
};

//! Parent class of AABB iterators that knows how to traverse general AABB tree structures.
class AABBIterator : public NeighborQueryPerPointIterator
{
public:
    //! Constructor
    AABBIterator(const AABBQuery* neighbor_query, const Vec3<float>& query_point,
                 unsigned int query_point_idx, float r_max, float r_min, bool exclude_ii)
        : NeighborQueryPerPointIterator(neighbor_query, query_point, query_point_idx, r_max, r_min,
                                        exclude_ii),
          m_aabb_query(neighbor_query)
    {}

    //! Empty Destructor
    ~AABBIterator() override = default;

    //! Computes the image vectors to query for
    void updateImageVectors(float r_max, bool _check_r_max = true);

protected:
    const AABBQuery* m_aabb_query;         //!< Link to the AABBQuery object
    std::vector<Vec3<float>> m_image_list; //!< List of translation vectors
    unsigned int m_n_images {0};           //!< The number of image vectors to check
};

//! Iterator that gets a specified number of nearest neighbors from AABB tree structures.
class AABBQueryIterator : public AABBIterator
{
public:
    //! Constructor
    AABBQueryIterator(const AABBQuery* neighbor_query, const Vec3<float>& query_point,
                      unsigned int query_point_idx, unsigned int num_neighbors, float r_guess, float r_max,
                      float r_min, float scale, bool exclude_ii)
        : AABBIterator(neighbor_query, query_point, query_point_idx, r_max, r_min, exclude_ii),
          m_num_neighbors(num_neighbors), m_r_cur(r_guess), m_scale(scale), m_all_bonds_minimum_distance(),
          m_query_points_below_r_min()
    {
        updateImageVectors(0);
    }

    //! Empty Destructor
    ~AABBQueryIterator() override = default;

    //! Get the next element.
    NeighborBond next() override;

protected:
    unsigned int m_count {0};                      //!< Number of neighbors returned for the current point.
    unsigned int m_num_neighbors;                  //!< Number of nearest neighbors to find
    std::vector<NeighborBond> m_current_neighbors; //!< The current set of found neighbors.
    bool m_search_extended {false}; //!< Flag to see whether we've gone past the safe cutoff distance and have
                                    //!< to be worried about finding duplicates.
    float
        m_r_cur; //!< Current search ball cutoff distance in use for the current particle (expands as needed).
    float m_scale; //!< The amount to scale m_r by when the current ball is too small.
    std::map<unsigned int, NeighborBond>
        m_all_bonds_minimum_distance; //!< Hash map of minimum distances found for a given point,
                                      //!< used when searching beyond maximum safe AABB distance.
    std::unordered_set<unsigned int> m_query_points_below_r_min; //!< The set of query_points that were too
                                                                 //!< close based on the r_min threshold.
};

//! Iterator that gets neighbors in a ball of size r_max using AABB tree structures.
class AABBQueryBallIterator : public AABBIterator
{
public:
    //! Constructor
    AABBQueryBallIterator(const AABBQuery* neighbor_query, const Vec3<float>& query_point,
                          unsigned int query_point_idx, float r_max, float r_min, bool exclude_ii,
                          bool _check_r_max = true)
        : AABBIterator(neighbor_query, query_point, query_point_idx, r_max, r_min, exclude_ii)
    {
        updateImageVectors(m_r_max, _check_r_max);
    }

    //! Empty Destructor
    ~AABBQueryBallIterator() override = default;

    //! Get the next element.
    NeighborBond next() override;

private:
    unsigned int cur_image {0};    //!< The current node in the tree.
    unsigned int cur_node_idx {0}; //!< The current node in the tree.
    unsigned int cur_ref_p {
        0}; //!< The current index into the reference particles in the current node of the tree.
};

}; }; // end namespace molcpp::locality

#endif // AABBQUERY_H