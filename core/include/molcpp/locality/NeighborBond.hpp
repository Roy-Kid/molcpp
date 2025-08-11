#ifndef MOLCPP_LOCALITY_NEIGHBORBOND_HPP
#define MOLCPP_LOCALITY_NEIGHBORBOND_HPP

#include <cmath>
#include "../types.hpp"

namespace molcpp {
namespace locality {

//! Structure representing a neighbor bond/pair
/*! A NeighborBond stores information about a pair of neighboring particles,
    including their indices, distance, weight, and distance vector.
*/
struct NeighborBond {
    unsigned int query_point_idx;  //!< Index of the query point
    unsigned int point_idx;        //!< Index of the neighbor point
    double distance;                //!< Distance between the points
    double weight;                  //!< Weight of the bond (default 1.0)
    Vec3 distance_vec;              //!< Distance vector from query to neighbor

    //! Default constructor
    NeighborBond() 
        : query_point_idx(0), point_idx(0), distance(0.0), weight(1.0) {
        distance_vec.fill(0.0);
    }

    //! Constructor with indices and distance
    NeighborBond(unsigned int qi, unsigned int pi, double d)
        : query_point_idx(qi), point_idx(pi), distance(d), weight(1.0) {
        distance_vec.fill(0.0);
    }

    //! Constructor with all parameters
    NeighborBond(unsigned int qi, unsigned int pi, double d, double w, const Vec3& dv)
        : query_point_idx(qi), point_idx(pi), distance(d), weight(w), distance_vec(dv) {}

    //! Less than operator for sorting
    bool operator<(const NeighborBond& other) const {
        if (query_point_idx != other.query_point_idx) {
            return query_point_idx < other.query_point_idx;
        }
        if (point_idx != other.point_idx) {
            return point_idx < other.point_idx;
        }
        return distance < other.distance;
    }

    //! Equality operator
    bool operator==(const NeighborBond& other) const {
        return query_point_idx == other.query_point_idx &&
               point_idx == other.point_idx &&
               std::abs(distance - other.distance) < 1e-10 &&
               std::abs(weight - other.weight) < 1e-10;
    }

    //! Inequality operator
    bool operator!=(const NeighborBond& other) const {
        return !(*this == other);
    }
};

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_NEIGHBORBOND_HPP