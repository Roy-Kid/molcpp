#ifndef MOLCPP_LOCALITY_AABBQUERY_HPP
#define MOLCPP_LOCALITY_AABBQUERY_HPP

#include <algorithm>
#include <cmath>
#include <vector>
#include <queue>
#include <xtensor/xarray.hpp>

#include "AABBTree.hpp"
#include "NeighborQuery.hpp"
#include "../box/Box.hpp"

namespace molcpp {
namespace locality {

//! AABB-based neighbor query implementation
/*! AABBQuery uses an AABB tree to efficiently find neighbors.
    It supports both ball queries and k-nearest neighbor queries.
*/
class AABBQuery : public NeighborQuery {
public:
    //! Default constructor
    AABBQuery() : NeighborQuery() {}

    //! Constructor with box and points
    AABBQuery(const box::Box& box, const xt::xarray<double>& points)
        : NeighborQuery(box, points) {
        buildTree();
    }

    //! Destructor
    ~AABBQuery() override = default;

    //! Build the AABB tree from current points
    void buildTree() {
        std::vector<AABB> aabbs;
        aabbs.reserve(m_n_points);
        
        for (size_t i = 0; i < m_n_points; ++i) {
            Vec3 pos;
            for (size_t j = 0; j < 3; ++j) {
                pos(j) = m_points(i, j);
            }
            aabbs.emplace_back(pos, static_cast<unsigned int>(i));
        }
        
        m_aabb_tree.buildTree(aabbs);
    }

    //! Get the AABB tree
    const AABBTree& getAABBTree() const { return m_aabb_tree; }

protected:
    //! Query for neighbors of a single point
    void queryPoint(const Vec3& query_point, unsigned int query_idx,
                   const QueryArgs& args, NeighborList& nlist) const override {
        if (args.mode == QueryType::ball) {
            queryBall(query_point, query_idx, args, nlist);
        } else {
            queryNearest(query_point, query_idx, args, nlist);
        }
    }

    //! Validate query arguments for AABB-specific requirements
    void validateQueryArgs(QueryArgs& args) const override {
        NeighborQuery::validateQueryArgs(args);
        
        if (args.mode == QueryType::nearest) {
            if (args.scale == DEFAULT_SCALE) {
                args.scale = 1.1;  // Default scale factor
            } else if (args.scale <= 1.0) {
                throw std::runtime_error("The scale query argument must be greater than 1.");
            }

            if (args.r_guess == DEFAULT_R_GUESS) {
                // Estimate based on uniform density
                double volume = m_box.getVolume();
                double density = static_cast<double>(m_n_points) / volume;
                double r_guess = std::cbrt((3.0 * args.num_neighbors) / (4.0 * M_PI * density));
                
                // Limit by box dimensions
                Vec3 nearest_plane = m_box.getNearestPlaneDistance();
                double min_plane = std::min({nearest_plane(0), nearest_plane(1), nearest_plane(2)});
                args.r_guess = std::min(r_guess, min_plane / 2.0);
            }
        }
    }

private:
    //! Perform a ball query
    void queryBall(const Vec3& query_point, unsigned int query_idx,
                  const QueryArgs& args, NeighborList& nlist) const {
        // Get all image vectors for periodic boundaries
        std::vector<Vec3> images = getImageVectors(query_point, args.r_max);
        
        for (const auto& image : images) {
            Vec3 shifted_point = query_point + image;
            
            // Create query AABB
            AABB query_aabb(shifted_point - Vec3{args.r_max, args.r_max, args.r_max},
                           shifted_point + Vec3{args.r_max, args.r_max, args.r_max});
            
            // Query the tree
            std::vector<unsigned int> candidates;
            m_aabb_tree.query(query_aabb, candidates);
            
            // Check actual distances
            for (unsigned int point_idx : candidates) {
                if (args.exclude_ii && point_idx == query_idx) {
                    continue;
                }
                
                Vec3 point_pos;
                for (size_t j = 0; j < 3; ++j) {
                    point_pos(j) = m_points(point_idx, j);
                }
                
                Vec3 dr = point_pos - shifted_point;
                double dist = std::sqrt(dr(0)*dr(0) + dr(1)*dr(1) + dr(2)*dr(2));
                
                if (dist >= args.r_min && dist <= args.r_max) {
                    nlist.addBond(query_idx, point_idx, dist, 1.0, dr);
                }
            }
        }
    }

    //! Perform a k-nearest neighbor query
    void queryNearest(const Vec3& query_point, unsigned int query_idx,
                     const QueryArgs& args, NeighborList& nlist) const {
        // Priority queue to keep k nearest neighbors
        using NeighborPair = std::pair<double, unsigned int>;
        std::priority_queue<NeighborPair> nearest;
        
        double r_search = args.r_guess;
        size_t found = 0;
        
        // Iteratively expand search radius until we find enough neighbors
        while (found < args.num_neighbors) {
            std::vector<Vec3> images = getImageVectors(query_point, r_search);
            
            for (const auto& image : images) {
                Vec3 shifted_point = query_point + image;
                
                // Create query sphere
                AABBSphere query_sphere(shifted_point, r_search);
                
                // Query the tree
                std::vector<unsigned int> candidates;
                m_aabb_tree.query(query_sphere, candidates);
                
                // Check actual distances
                for (unsigned int point_idx : candidates) {
                    if (args.exclude_ii && point_idx == query_idx) {
                        continue;
                    }
                    
                    Vec3 point_pos;
                    for (size_t j = 0; j < 3; ++j) {
                        point_pos(j) = m_points(point_idx, j);
                    }
                    
                    Vec3 dr = point_pos - shifted_point;
                    double dist = std::sqrt(dr(0)*dr(0) + dr(1)*dr(1) + dr(2)*dr(2));
                    
                    if (dist <= r_search) {
                        if (nearest.size() < args.num_neighbors) {
                            nearest.push({dist, point_idx});
                        } else if (dist < nearest.top().first) {
                            nearest.pop();
                            nearest.push({dist, point_idx});
                        }
                    }
                }
            }
            
            found = nearest.size();
            if (found < args.num_neighbors) {
                r_search *= args.scale;  // Expand search radius
            }
        }
        
        // Extract results from priority queue
        std::vector<NeighborPair> results;
        while (!nearest.empty()) {
            results.push_back(nearest.top());
            nearest.pop();
        }
        
        // Add to neighbor list in reverse order (smallest distance first)
        for (auto it = results.rbegin(); it != results.rend(); ++it) {
            Vec3 point_pos;
            for (size_t j = 0; j < 3; ++j) {
                point_pos(j) = m_points(it->second, j);
            }
            Vec3 dr = m_box.minimumImage(query_point, point_pos);
            nlist.addBond(query_idx, it->second, it->first, 1.0, dr);
        }
    }

    //! Get image vectors for periodic boundaries
    std::vector<Vec3> getImageVectors(const Vec3& query_point, double r_max) const {
        std::vector<Vec3> images;
        images.push_back(Vec3{0.0, 0.0, 0.0});  // Original position
        
        if (!m_box.isPeriodicX() && !m_box.isPeriodicY() && !m_box.isPeriodicZ()) {
            return images;  // No periodic boundaries
        }
        
        // Calculate number of images needed in each direction
        int nx = m_box.isPeriodicX() ? static_cast<int>(std::ceil(r_max / m_box.getLx())) : 0;
        int ny = m_box.isPeriodicY() ? static_cast<int>(std::ceil(r_max / m_box.getLy())) : 0;
        int nz = m_box.isPeriodicZ() ? static_cast<int>(std::ceil(r_max / m_box.getLz())) : 0;
        
        Mat3 box_vectors = m_box.getVectors();
        
        for (int ix = -nx; ix <= nx; ++ix) {
            for (int iy = -ny; iy <= ny; ++iy) {
                for (int iz = -nz; iz <= nz; ++iz) {
                    if (ix == 0 && iy == 0 && iz == 0) continue;  // Skip origin
                    
                    Vec3 image;
                    for (size_t i = 0; i < 3; ++i) {
                        image(i) = ix * box_vectors(i, 0) + 
                                  iy * box_vectors(i, 1) + 
                                  iz * box_vectors(i, 2);
                    }
                    images.push_back(image);
                }
            }
        }
        
        return images;
    }

    AABBTree m_aabb_tree;  //!< The AABB tree for efficient queries
};

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_AABBQUERY_HPP