#ifndef MOLCPP_LOCALITY_NEIGHBORQUERY_HPP
#define MOLCPP_LOCALITY_NEIGHBORQUERY_HPP

#include <memory>
#include <vector>
#include <limits>
#include <stdexcept>
#include <xtensor/xarray.hpp>

#include "NeighborList.hpp"
#include "../spatial/box.hpp"
#include "../types.hpp"

namespace molcpp {
namespace locality {

//! Enum for query types
enum class QueryType {
    ball,      //!< Find all neighbors within a radius
    nearest    //!< Find k nearest neighbors
};

//! Default values for query arguments
constexpr double DEFAULT_R_MAX = std::numeric_limits<double>::infinity();
constexpr double DEFAULT_R_MIN = 0.0;
constexpr double DEFAULT_SCALE = -1.0;
constexpr double DEFAULT_R_GUESS = -1.0;
constexpr unsigned int DEFAULT_NUM_NEIGHBORS = std::numeric_limits<unsigned int>::max();

//! Structure containing query arguments
struct QueryArgs {
    QueryType mode = QueryType::ball;           //!< Query mode
    double r_max = DEFAULT_R_MAX;               //!< Maximum radius for ball query
    double r_min = DEFAULT_R_MIN;               //!< Minimum radius
    double scale = DEFAULT_SCALE;               //!< Scale factor for nearest neighbor search
    double r_guess = DEFAULT_R_GUESS;           //!< Initial radius guess for nearest neighbor
    unsigned int num_neighbors = DEFAULT_NUM_NEIGHBORS;  //!< Number of neighbors for nearest query
    bool exclude_ii = false;                    //!< Exclude self-neighbors

    //! Constructor for ball query
    static QueryArgs ball(double r_max, double r_min = 0.0, bool exclude_ii = false) {
        QueryArgs args;
        args.mode = QueryType::ball;
        args.r_max = r_max;
        args.r_min = r_min;
        args.exclude_ii = exclude_ii;
        return args;
    }

    //! Constructor for nearest neighbor query
    static QueryArgs nearest(unsigned int k, double r_guess = DEFAULT_R_GUESS, 
                            double scale = DEFAULT_SCALE, bool exclude_ii = true) {
        QueryArgs args;
        args.mode = QueryType::nearest;
        args.num_neighbors = k;
        args.r_guess = r_guess;
        args.scale = scale;
        args.exclude_ii = exclude_ii;
        return args;
    }
};

//! Abstract base class for neighbor query methods
/*! NeighborQuery provides an interface for different neighbor finding algorithms.
    Derived classes implement specific methods like AABB trees, link cells, etc.
*/
class NeighborQuery {
public:
    //! Constructor
    NeighborQuery() : m_n_points(0) {}

    //! Constructor with box and points
    NeighborQuery(const Box& box, const xt::xarray<double>& points)
        : m_box(box), m_points(points), m_n_points(points.shape(0)) {
        if (points.dimension() != 2 || points.shape(1) != 3) {
            throw std::invalid_argument("Points must be an Nx3 array");
        }
    }

    //! Virtual destructor
    virtual ~NeighborQuery() = default;

    //! Query for neighbors
    /*! \param query_points Points to query neighbors for
        \param args Query arguments
        \returns NeighborList containing all neighbor pairs
    */
    virtual NeighborList query(const xt::xarray<double>& query_points, 
                              const QueryArgs& args) const {
        validateQueryArgs(const_cast<QueryArgs&>(args));
        
        if (query_points.dimension() != 2 || query_points.shape(1) != 3) {
            throw std::invalid_argument("Query points must be an Nx3 array");
        }

        NeighborList nlist;
        size_t n_query = query_points.shape(0);

        // Process each query point
        #ifdef _OPENMP
        #pragma omp parallel
        {
            NeighborList local_nlist;
            #pragma omp for schedule(dynamic)
            for (size_t i = 0; i < n_query; ++i) {
                Vec3 query_point;
                for (size_t j = 0; j < 3; ++j) {
                    query_point(j) = query_points(i, j);
                }
                queryPoint(query_point, i, args, local_nlist);
            }
            #pragma omp critical
            {
                for (const auto& bond : local_nlist) {
                    nlist.addBond(bond);
                }
            }
        }
        #else
        for (size_t i = 0; i < n_query; ++i) {
            Vec3 query_point;
            for (size_t j = 0; j < 3; ++j) {
                query_point(j) = query_points(i, j);
            }
            queryPoint(query_point, i, args, nlist);
        }
        #endif

        return nlist;
    }

    //! Get the box
    const Box& getBox() const { return m_box; }

    //! Get the points
    const xt::xarray<double>& getPoints() const { return m_points; }

    //! Get the number of points
    size_t getNPoints() const { return m_n_points; }

protected:
    //! Query for neighbors of a single point (to be implemented by derived classes)
    virtual void queryPoint(const Vec3& query_point, unsigned int query_idx,
                           const QueryArgs& args, NeighborList& nlist) const = 0;

    //! Validate and adjust query arguments
    virtual void validateQueryArgs(QueryArgs& args) const {
        if (args.mode == QueryType::ball) {
            if (args.r_max < 0) {
                throw std::invalid_argument("r_max must be non-negative");
            }
            if (args.r_min < 0) {
                throw std::invalid_argument("r_min must be non-negative");
            }
            if (args.r_min > args.r_max) {
                throw std::invalid_argument("r_min must be less than or equal to r_max");
            }
        } else if (args.mode == QueryType::nearest) {
            if (args.num_neighbors == 0) {
                throw std::invalid_argument("num_neighbors must be positive");
            }
            if (args.num_neighbors == DEFAULT_NUM_NEIGHBORS) {
                args.num_neighbors = 1;  // Default to 1 nearest neighbor
            }
        }
    }

    Box m_box;                    //!< Simulation box
    xt::xarray<double> m_points;       //!< Point positions
    size_t m_n_points;                 //!< Number of points
};

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_NEIGHBORQUERY_HPP