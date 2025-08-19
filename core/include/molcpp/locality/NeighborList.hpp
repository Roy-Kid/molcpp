// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef NEIGHBOR_LIST_H
#define NEIGHBOR_LIST_H

#include <cstddef>
#include <memory>
#include <vector>
#include <xtensor/containers/xarray.hpp>

#include "molcpp/spatial/box.hpp"
#include "NeighborBond.hpp"
#include "molcpp/types.hpp"

namespace molcpp { namespace locality {

//! Store a number of near-neighbor bonds from one set of positions
//  ("query points") to another set ("points")
/*! A NeighborList object acts as a source of neighbor information for
    molcpp's compute methods. Briefly, each bond is associated with a
    query point index, a point index, a distance, and a weight.

    <b>Data structures:</b>

    Query point and point indices are stored in a 2D array _neighbors of shape
    (n_bonds, 2). The distances and weights arrays are flat per-bond arrays.
 */
class NeighborList
{
public:
    //! Default constructor
    NeighborList();
    //! Create a NeighborList that can hold up to the given number of bonds
    explicit NeighborList(unsigned int num_bonds);
    //! Copy constructor (makes a deep copy)
    NeighborList(const NeighborList& other);
    //! Construct from arrays
    NeighborList(unsigned int num_bonds, const unsigned int* query_point_index, unsigned int num_query_points,
                 const unsigned int* point_index, unsigned int num_points, const Vec3<float>* vectors,
                 const float* weights);
    //! Make a neighborlist where all points, excluding ii, are pairs
    NeighborList(const Vec3<float>* points, const Vec3<float>* query_points, const Box& box,
                 const bool exclude_ii, const unsigned int num_points, const unsigned int num_query_points);

    //! Construct from vector of NeighborBonds
    explicit NeighborList(std::vector<NeighborBond> bonds);

    //! Return the number of bonds stored in this NeighborList
    unsigned int getNumBonds() const;
    //! Return the number of query points this NeighborList was built with
    unsigned int getNumQueryPoints() const;
    //! Return the number of points this NeighborList was built with
    unsigned int getNumPoints() const;

    //! Set the number of bonds, query points, and points for this NeighborList object
    void setNumBonds(unsigned int num_bonds, unsigned int num_query_points, unsigned int num_points);
    //! Update the arrays of neighbor counts and segments
    void updateSegmentCounts() const;

    //! Access the neighbors array for reading
    std::shared_ptr<const xt::xarray<unsigned int>> getNeighbors() const
    {
        return _neighbors;
    }

    //! Access the distances array for reading
    std::shared_ptr<const xt::xarray<float>> getDistances() const
    {
        return _distances;
    }

    //! Access the weights array for reading
    std::shared_ptr<const xt::xarray<float>> getWeights() const
    {
        return _weights;
    }

    //! Access the vectors array for reading
    std::shared_ptr<const xt::xarray<Vec3<float>>> getVectors() const
    {
        return _vectors;
    }

    //! Access the counts array for reading
    std::shared_ptr<const xt::xarray<unsigned int>> getCounts() const
    {
        updateSegmentCounts();
        return _counts;
    }
    //! Access the segments array for reading
    std::shared_ptr<const xt::xarray<unsigned int>> getSegments() const
    {
        updateSegmentCounts();
        return _segments;
    }

    /**
     * Set the values for the neighbor index to be that of the given neighborbond
     */
    void setNeighborEntry(size_t neighbor_index, const NeighborBond& nb)
    {
        (*_neighbors)(neighbor_index, 0) = nb.getQueryPointIdx();
        (*_neighbors)(neighbor_index, 1) = nb.getPointIdx();
        (*_vectors)[neighbor_index] = nb.getVector();
        (*_distances)[neighbor_index] = nb.getDistance();
        (*_weights)[neighbor_index] = nb.getWeight();
    }

    //! Remove bonds in this object based on an array of boolean values. The
    //  array must be at least as long as the number of neighbor bonds.
    //  Returns the number of bonds removed.
    template<typename Iterator> unsigned int filter(Iterator begin);
    //! Remove bonds in this object based on minimum and maximum distance
    //  constraints. Returns the number of bonds removed.
    unsigned int filterR(float r_max, float r_min = 0);

    //! Return the first bond index corresponding to point i
    unsigned int findFirstIndex(unsigned int i) const;

    //! Resize member arrays to a different size
    void resize(unsigned int num_bonds);

    //! Copy the bonds from another NeighborList object
    void copy(const NeighborList& other);
    //! Throw a runtime_error if num_points and num_query_points do not match
    //  the stored value
    void validate(unsigned int num_query_points, unsigned int num_points) const;
    // sort the neighborlist
    void sort(bool by_distance);

    //! Helper method to get an equivalent list of NeighborBonds from the nlist
    std::vector<NeighborBond> toBondVector() const;

private:
    //! Helper method for bisection search of the neighbor list, used in findFirstIndex
    unsigned int bisectionSearch(unsigned int val, unsigned int left, unsigned int right) const;

    //! Number of query points
    unsigned int _num_query_points;
    //! Number of points
    unsigned int _num_points;
    //! Neighbor list indices array
    std::shared_ptr<xt::xarray<unsigned int>> _neighbors;
    //! Neighbor list per-bond distance array
    std::shared_ptr<xt::xarray<float>> _distances;
    //! Neighbor list per-bond weight array
    std::shared_ptr<xt::xarray<float>> _weights;
    //!< Directed vectors per-bond array
    std::shared_ptr<xt::xarray<Vec3<float>>> _vectors;

    //! Track whether segments and counts are up to date
    mutable bool _segments_counts_updated;
    //! Neighbor counts for each query point
    mutable std::shared_ptr<xt::xarray<unsigned int>> _counts;
    //! Neighbor segments for each query point
    mutable std::shared_ptr<xt::xarray<unsigned int>> _segments;
};

bool compareNeighborBond(const NeighborBond& left, const NeighborBond& right);
bool compareNeighborDistance(const NeighborBond& left, const NeighborBond& right);
bool compareFirstNeighborPairs(const std::vector<NeighborBond>& left, const std::vector<NeighborBond>& right);

}; }; // end namespace molcpp::locality

#endif // NEIGHBOR_LIST_H