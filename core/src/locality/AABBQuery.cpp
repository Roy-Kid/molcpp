// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "molcpp/locality/AABB.hpp"
#include "molcpp/locality/AABBQuery.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/locality/NeighborBond.hpp"
#include "molcpp/locality/NeighborQuery.hpp"
#include "molcpp/types.hpp"
#include "xtensor/views/xview.hpp"

namespace molcpp { namespace locality {

AABBQuery::AABBQuery(const Box& box, const XYZ& points)
    : NeighborQuery(box, points)
{
    // Build the initial neighborlist
    build(box, points);
}

AABBQuery::~AABBQuery() = default;

std::shared_ptr<NeighborQueryPerPointIterator>
AABBQuery::querySingle(const Vec3<float> query_point, unsigned int query_point_idx, QueryArgs args) const
{
    this->validateQueryArgs(args);
    if (args.mode == QueryType::ball)
    {
        return std::make_shared<AABBQueryBallIterator>(this, query_point, query_point_idx, args.r_max,
                                                       args.r_min, args.exclude_ii);
    }
    if (args.mode == QueryType::nearest)
    {
        return std::make_shared<AABBQueryIterator>(this, query_point, query_point_idx, args.num_neighbors,
                                                   args.r_guess, args.r_max, args.r_min, args.scale,
                                                   args.exclude_ii);
    }
    throw std::runtime_error("Invalid query mode provided to query function in AABBQuery.");
}

void AABBQuery::build(const Box& box, const XYZ& points)
{
    // Update base class members
    setBox(box);
    setPoints(points);
    
    // Allocate memory and create image vectors
    setupTree(_n_points);

    // Build the tree
    buildTree(points);
}

void AABBQuery::update(const XYZ& points)
{
    // Use base class method to add new points
    addPoints(points);
    
    // Resize _aabbs to match the new number of points
    setupTree(_n_points);
    
    // Rebuild the AABB tree with all points
    buildTree(_points);
}

void AABBQuery::setupTree(unsigned int Np)
{
    _aabbs.resize(Np);
}

void AABBQuery::buildTree(const XYZ& points)
{
    unsigned int Np = points.shape(0);
    
    // Construct a point AABB for each point
    for (unsigned int i = 0; i < Np; ++i)
    {
        // Make a point AABB - extract Vec3 from XYZ array
        Vec3<float> my_pos;
        my_pos[0] = points(i, 0);
        my_pos[1] = points(i, 1);
        my_pos[2] = points(i, 2);
        _aabbs[i] = AABB(my_pos, i);
    }
    
    // Call the tree build routine, one tree per type
    _aabb_tree.buildTree(_aabbs.data(), Np);
}

void AABBIterator::updateImageVectors(float r_max, bool _check_r_max)
{
    Box const& box = m_neighbor_query->getBox();
    Vec3<float> const nearest_plane_distance = box.getNearestPlaneDistance();
    Vec3<bool> const periodic = box.pbc();
    if (_check_r_max)
    {
        if ((periodic[0] && nearest_plane_distance[0] <= r_max * 2.0)
            || (periodic[1] && nearest_plane_distance[1] <= r_max * 2.0)
            || (periodic[2] && nearest_plane_distance[2] <= r_max * 2.0))
        {
            throw std::runtime_error("The AABBQuery r_max is too large for this box.");
        }
    }

    // Now compute the image vectors
    // Each dimension increases by one power of 3
    unsigned int const n_dim_periodic = static_cast<unsigned int>(periodic[0])
        + static_cast<unsigned int>(periodic[1])
        + static_cast<unsigned int>(periodic[2]);
    m_n_images = 1;
    for (unsigned int dim = 0; dim < n_dim_periodic; ++dim)
    {
        m_n_images *= 3;
    }

    // Reallocate memory if necessary
    if (m_n_images > m_image_list.size())
    {
        m_image_list.resize(m_n_images);
    }

    auto latt_a = Vec3<float>(box.getLatticeVector(0));
    auto latt_b = Vec3<float>(box.getLatticeVector(1));
    Vec3<float> latt_c = Vec3<float>(box.getLatticeVector(2));

    // There is always at least 1 image, which we put as our first thing to look at
    m_image_list[0] = Vec3<float>{0.0f, 0.0f, 0.0f};

    // Iterate over all other combinations of images
    unsigned int n_images = 1;
    for (int i = -1; i <= 1 && n_images < m_n_images; ++i)
    {
        for (int j = -1; j <= 1 && n_images < m_n_images; ++j)
        {
            for (int k = -1; k <= 1 && n_images < m_n_images; ++k)
            {
                if (i != 0 || j != 0 || k != 0)
                {
                    // Skip any periodic images if we don't have periodicity
                    if ((i != 0 && !periodic[0]) || (j != 0 && !periodic[1])
                        || (k != 0 && !periodic[2]))
                    {
                        continue;
                    }

                    m_image_list[n_images] = float(i) * latt_a + float(j) * latt_b + float(k) * latt_c;
                    ++n_images;
                }
            }
        }
    }
}

NeighborBond AABBQueryBallIterator::next()
{
    float const r_max_sq = m_r_max * m_r_max;
    float const r_min_sq = m_r_min * m_r_min;

    // Read in the position of current point
    Vec3<float> pos_i(m_query_point);

    // Loop over image vectors
    while (cur_image < m_n_images)
    {
        // Make an AABB for the image of this point
        Vec3<float> const pos_i_image = pos_i + m_image_list[cur_image];
        AABBSphere const asphere = AABBSphere(pos_i_image, m_r_max);

        // Stackless traversal of the tree
        while (cur_node_idx < m_aabb_query->getAABBTree().getNumNodes())
        {
            if (overlap(m_aabb_query->getAABBTree().getNodeAABB(cur_node_idx), asphere))
            {
                if (m_aabb_query->getAABBTree().isNodeLeaf(cur_node_idx))
                {
                    while (cur_ref_p < m_aabb_query->getAABBTree().getNodeNumParticles(cur_node_idx))
                    {
                        // Neighbor j
                        const unsigned int j
                            = m_aabb_query->getAABBTree().getNodeParticleTag(cur_node_idx, cur_ref_p);
                        // Increment before possible return.
                        cur_ref_p++;

                        // Skip ii matches immediately if requested.
                        if (m_exclude_ii && m_query_point_idx == j)
                        {
                            continue;
                        }

                        // Read in the position of j
                        Vec3<float> pos_j((*m_neighbor_query)[j]);

                        // Compute distance
                        const Vec3<float> r_ij = pos_j - pos_i_image;
                        const float r_sq = xt::sum(r_ij * r_ij)();

                        // Check ii exclusion before including the pair.
                        if (r_sq < r_max_sq && r_sq >= r_min_sq)
                        {
                            return NeighborBond(m_query_point_idx, j, std::sqrt(r_sq), 1, r_ij);
                        }
                    }
                }
            }
            else
            {
                // Skip ahead
                cur_node_idx += m_aabb_query->getAABBTree().getNodeSkip(cur_node_idx);
            }
            cur_node_idx++;
            cur_ref_p = 0;
        } // end stackless search
        cur_image++;
        cur_node_idx = 0;
    } // end loop over images

    m_finished = true;
    return NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f});
}

NeighborBond AABBQueryIterator::next()
{
    Vec3<float> const plane_distance = m_neighbor_query->getBox().getNearestPlaneDistance();
    float min_plane_distance = std::min({plane_distance[0], plane_distance[1], plane_distance[2]});
    float max_plane_distance = std::max({plane_distance[0], plane_distance[1], plane_distance[2]});

    // This iterator is not truly lazy; because it needs to get possible
    // neighbors and sort them to find the actual ones, it computes and caches
    // them and then returns them one-by-one. This check ensures that we only
    // search for new neighbors the first time next is called.
    if (m_current_neighbors.empty())
    {
        // Continually perform ball queries until the termination conditions are met.
        while (true)
        {
            // Perform a ball query to get neighbors. To ensure that we allow
            // ball queries to exceed their normal boundaries, we pass false as
            // the _check_r_max. We also can't depend on the ball query for
            // r_min filtering because we're querying beyond the normally safe
            // bounds, so we have to do it in this class.
            m_current_neighbors.clear();
            m_all_bonds_minimum_distance.clear();
            m_query_points_below_r_min.clear();
            std::shared_ptr<NeighborQueryPerPointIterator> const ball_it
                = std::make_shared<AABBQueryBallIterator>(static_cast<const AABBQuery*>(m_neighbor_query),
                                                          m_query_point, m_query_point_idx,
                                                          std::min(m_r_cur, m_r_max), 0, m_exclude_ii, false);
            while (!ball_it->end())
            {
                NeighborBond nb = ball_it->next();
                if (nb == NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f}))
                {
                    continue;
                }

                if (!m_exclude_ii || m_query_point_idx != nb.getPointIdx())
                {
                    nb.setQueryPointIdx(m_query_point_idx);
                    // If we've expanded our search radius beyond safe
                    // distance, use the map instead of the vector.
                    if (m_search_extended)
                    {
                        const unsigned int nb_point_idx = nb.getPointIdx();
                        const float nb_distance = nb.getDistance();
                        if ((m_all_bonds_minimum_distance.count(nb_point_idx) == 0)
                            || m_all_bonds_minimum_distance[nb_point_idx].getDistance() > nb_distance)
                        {
                            m_all_bonds_minimum_distance[nb_point_idx] = nb;
                            if (nb_distance < m_r_min)
                            {
                                m_query_points_below_r_min.insert(nb_point_idx);
                            }
                        }
                    }
                    else
                    {
                        if (nb.getDistance() >= m_r_min)
                        {
                            m_current_neighbors.emplace_back(nb);
                        }
                    }
                }
            }

            // Break if there are enough neighbors, or if we are querying beyond the limits of
            // the periodic box.
            m_r_cur *= m_scale;

            if (m_current_neighbors.size() >= m_num_neighbors)
            {
                std::sort(m_current_neighbors.begin(), m_current_neighbors.end());
                break;
            }

            if ((m_r_cur >= m_r_max) || (m_r_cur >= max_plane_distance)
                || ((m_all_bonds_minimum_distance.size() - m_query_points_below_r_min.size())
                    >= m_num_neighbors))
            {
                // Once this condition is reached, either we found enough
                // neighbors beyond the normal min_plane_distance
                // condition or we conclude that there are not enough
                // neighbors left in the system.
                for (const auto& minimum_distance_bond : m_all_bonds_minimum_distance)
                {
                    if (minimum_distance_bond.second.getDistance() >= m_r_min)
                    {
                        m_current_neighbors.emplace_back(minimum_distance_bond.second);
                    }
                }
                std::sort(m_current_neighbors.begin(), m_current_neighbors.end());
                break;
            }

            if (m_r_cur > min_plane_distance / 2)
            {
                // If we have to go beyond the cutoff radius, we need to
                // start tracking what particles are already in the set so
                // that we can make sure that we find the closest image
                // because we now run the risk of finding duplicates.
                //
                // We could make this marginally more efficient by checking
                // whether we've exactly hit the limit, or if there's a
                // rescaling that would let us try the exact limit once
                // before going beyond the min plane distance.
                m_search_extended = true;
            }
        }
    }

    // Now we return all the points found for the current point, stopping when
    // any are beyond the maximum distance allowed.
    while ((m_count < m_num_neighbors) && (m_count < m_current_neighbors.size()))
    {
        m_count++;
        if (m_current_neighbors[m_count - 1].getDistance() > m_r_max)
        {
            m_finished = true;
            return NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f});
        }
        return m_current_neighbors[m_count - 1];
    }

    m_finished = true;
    return NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f});
}

}; }; // end namespace molcpp::locality