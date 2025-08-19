// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <xtensor/containers/xarray.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include "molcpp/spatial/box.hpp"
#include "molcpp/locality/NeighborList.hpp"
#include "molcpp/types.hpp"

namespace molcpp { namespace locality {

NeighborList::NeighborList()
    : _num_query_points(0), _num_points(0), _segments_counts_updated(false),
      _neighbors(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{0, 2})),
      _distances(std::make_shared<xt::xarray<float>>(std::vector<size_t>{0})),
      _weights(std::make_shared<xt::xarray<float>>(std::vector<size_t>{0})),
      _vectors(std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{0})),
      _segments(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{0})),
      _counts(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{0}))
{}

NeighborList::NeighborList(unsigned int num_bonds)
    : _num_query_points(0), _num_points(0), _segments_counts_updated(false),
      _neighbors(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_bonds, 2})),
      _distances(std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds})),
      _weights(std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds})),
      _vectors(std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{num_bonds})),
      _segments(std::make_shared<xt::xarray<unsigned int>>()),
      _counts(std::make_shared<xt::xarray<unsigned int>>())
{}

NeighborList::NeighborList(const NeighborList& other)
    : _num_query_points(other._num_query_points), _num_points(other._num_points),
      _segments_counts_updated(false)
{
    copy(other);
}

NeighborList::NeighborList(unsigned int num_bonds, const unsigned int* query_point_index,
                           unsigned int num_query_points, const unsigned int* point_index,
                           unsigned int num_points, const Vec3<float>* vectors, const float* weights)
    : _num_query_points(num_query_points), _num_points(num_points), _segments_counts_updated(false),
      _neighbors(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_bonds, 2})),
      _distances(std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds})),
      _weights(std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds})),
      _vectors(std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{num_bonds})),
      _segments(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_query_points})),
      _counts(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_query_points}))
{
    unsigned int last_index(0);
    for (unsigned int i = 0; i < num_bonds; i++)
    {
        unsigned int const index = query_point_index[i];
        if (index < last_index)
        {
            throw std::invalid_argument("NeighborList query_point_index must be sorted.");
        }
        if (index >= _num_query_points)
        {
            throw std::invalid_argument(
                "NeighborList query_point_index values must be less than num_query_points.");
        }
        if (point_index[i] >= _num_points)
        {
            throw std::invalid_argument("NeighborList point_index values must be less than num_points.");
        }
        setNeighborEntry(i, NeighborBond(index, point_index[i], weights[i], vectors[i]));
        last_index = index;
    }
}

NeighborList::NeighborList(const Vec3<float>* points, const Vec3<float>* query_points, const Box& box,
                           const bool exclude_ii, const unsigned int num_points,
                           const unsigned int num_query_points)
    : _num_points(num_points), _num_query_points(num_query_points), _segments_counts_updated(false),
      _segments(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_query_points})),
      _counts(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_query_points}))
{
    // prepare member arrays
    const unsigned int num_ii = (exclude_ii ? std::min(num_points, num_query_points) : 0);
    const unsigned int num_bonds = num_points * num_query_points - num_ii;

    _neighbors = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_bonds, 2});
    _distances = std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds});
    _weights = std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds});
    _vectors = std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{num_bonds});

    // Simple loop implementation since we don't have the freud utility functions
    for (unsigned int i = 0; i < num_query_points; ++i)
    {
        // set the starting value of the bond index
        unsigned int bond_idx = i * num_points;
        if (exclude_ii)
        {
            bond_idx -= std::min(i, num_points);
        }

        // loop over points
        for (unsigned int j = 0; j < num_points; ++j)
        {
            if (exclude_ii && i == j)
            {
                continue;
            }

            (*_neighbors)(bond_idx, 0) = i;
            (*_neighbors)(bond_idx, 1) = j;
            (*_weights)(bond_idx) = 1.0;
            const auto dr = box.delta(query_points[i], points[j]);
            (*_distances)(bond_idx) = std::sqrt(xt::sum(dr * dr)());
            (*_vectors)(bond_idx) = dr;
            ++bond_idx;
        }
    }
}

NeighborList::NeighborList(std::vector<NeighborBond> bonds)
    : _distances(std::make_shared<xt::xarray<float>>(std::vector<size_t>{bonds.size()})),
      _vectors(std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{bonds.size()})),
      _weights(std::make_shared<xt::xarray<float>>(std::vector<size_t>{bonds.size()})),
      _neighbors(std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{bonds.size(), 2}))
{
    // keep track of maximum indices
    unsigned int max_idx_query = 0;
    unsigned int max_idx_point = 0;

    // fill arrays in single-threaded implementation
    for (size_t i = 0; i < bonds.size(); ++i)
    {
        auto bond = bonds[i];

        // update max bond indices
        if (max_idx_point < bond.getPointIdx())
        {
            max_idx_point = bond.getPointIdx();
        }
        if (max_idx_query < bond.getQueryPointIdx())
        {
            max_idx_query = bond.getQueryPointIdx();
        }

        // fill in array data
        (*_distances)(i) = bond.getDistance();
        (*_weights)(i) = bond.getWeight();
        (*_neighbors)(i, 0) = bond.getQueryPointIdx();
        (*_neighbors)(i, 1) = bond.getPointIdx();
        (*_vectors)(i) = bond.getVector();
    }

    // set num points, query points as max of indices
    _num_points = max_idx_point + 1;
    _num_query_points = max_idx_query + 1;
    _segments = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
    _counts = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
    _segments_counts_updated = false;
}

unsigned int NeighborList::getNumBonds() const
{
    return _neighbors->shape()[0];
}

unsigned int NeighborList::getNumQueryPoints() const
{
    return _num_query_points;
}

unsigned int NeighborList::getNumPoints() const
{
    return _num_points;
}

void NeighborList::setNumBonds(unsigned int num_bonds, unsigned int num_query_points, unsigned int num_points)
{
    resize(num_bonds);
    _num_query_points = num_query_points;
    _num_points = num_points;
    _segments_counts_updated = false;
}

void NeighborList::updateSegmentCounts() const
{
    if (!_segments_counts_updated)
    {
        _counts = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
        _segments = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
        const unsigned int INDEX_TERMINATOR(0xffffffff);
        unsigned int last_index(INDEX_TERMINATOR);
        unsigned int counter(0);
        for (unsigned int i = 0; i < getNumBonds(); i++)
        {
            const unsigned int index((*_neighbors)(i, 0));
            if (index != last_index)
            {
                (*_segments)[index] = i;
                if (index > 0)
                {
                    if (last_index != INDEX_TERMINATOR)
                    {
                        (*_counts)[last_index] = counter;
                    }
                    counter = 0;
                }
            }
            last_index = index;
            counter++;
        }
        if (last_index != INDEX_TERMINATOR)
        {
            (*_counts)[last_index] = counter;
        }
        _segments_counts_updated = true;
    }
}

// We are currently assuming that the input iterator has the correct length;
// however, this is compatible with the original assumptions of this function
// (pre-iterator syntax), so we'll accept that level of type-safety for now. In
// the future, if we expose a more appropriate iterator API then we'll need to
// accept an "end" parameter as well.
template<typename Iterator> unsigned int NeighborList::filter(Iterator begin)
{
    const unsigned int old_size(getNumBonds());
    const auto end = begin + old_size;

    // new_size is the number of good (unfiltered-out) elements
    const unsigned int new_size(std::count(begin, end, true));

    // Arrays to hold filtered data - we use new arrays instead of writing over
    // existing data to avoid requiring a second pass in resize().
    auto new_neighbors
        = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{new_size, 2});
    auto new_distances = std::make_shared<xt::xarray<float>>(std::vector<size_t>{new_size});
    auto new_weights = std::make_shared<xt::xarray<float>>(std::vector<size_t>{new_size});
    auto new_vectors = std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{new_size});

    auto current_element = begin;
    unsigned int num_good(0);
    unsigned int max_query_idx = 0;
    unsigned int max_point_idx = 0;
    
    for (unsigned int i(0); i < old_size; ++i)
    {
        if (*current_element)
        {
            (*new_neighbors)(num_good, 0) = (*_neighbors)(i, 0);
            (*new_neighbors)(num_good, 1) = (*_neighbors)(i, 1);
            (*new_distances)[num_good] = (*_distances)[i];
            (*new_weights)[num_good] = (*_weights)[i];
            (*new_vectors)[num_good] = (*_vectors)[i];
            
            // Update max indices for recalculating _num_query_points and _num_points
            if ((*_neighbors)(i, 0) > max_query_idx) max_query_idx = (*_neighbors)(i, 0);
            if ((*_neighbors)(i, 1) > max_point_idx) max_point_idx = (*_neighbors)(i, 1);
            
            ++num_good;
        }
        ++current_element;
    }

    _neighbors = new_neighbors;
    _distances = new_distances;
    _weights = new_weights;
    _vectors = new_vectors;
    
    // Update counts based on filtered data
    _num_query_points = max_query_idx + 1;
    _num_points = max_point_idx + 1;
    
    // Reset segments and counts since they're no longer valid
    _segments = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
    _counts = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{_num_query_points});
    _segments_counts_updated = false;
    
    return old_size - new_size;
}

// Explicit template instantiation required for
// TODO not sure if explicitly instantiation is needed for nanobind
template unsigned int NeighborList::filter(std::vector<bool>::const_iterator);
template unsigned int NeighborList::filter(std::vector<bool>::iterator);
template unsigned int NeighborList::filter(const bool*);
template unsigned int NeighborList::filter(bool*);

unsigned int NeighborList::filterR(float r_max, float r_min)
{
    if (r_max <= 0)
    {
        throw std::invalid_argument("NeighborList.filterR requires r_max to be positive.");
    }
    if (r_min < 0)
    {
        throw std::invalid_argument("NeighborList.filterR requires r_min to be non-negative.");
    }
    if (r_max <= r_min)
    {
        throw std::invalid_argument("NeighborList.filterR requires that r_max must be greater than r_min.");
    }

    std::vector<bool> dist_filter(getNumBonds());
    for (unsigned int i(0); i < getNumBonds(); ++i)
    {
        dist_filter[i] = ((*_distances)[i] >= r_min && (*_distances)[i] < r_max);
    }
    return filter(dist_filter.cbegin());
}

unsigned int NeighborList::findFirstIndex(unsigned int i) const
{
    if (getNumBonds() != 0)
    {
        return bisectionSearch(i, 0, getNumBonds()) + (i > (*_neighbors)(0, 0) ? 1 : 0);
    }
    return 0;
}

void NeighborList::resize(unsigned int num_bonds)
{
    auto new_neighbors
        = std::make_shared<xt::xarray<unsigned int>>(std::vector<size_t>{num_bonds, 2});
    auto new_distances = std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds});
    auto new_weights = std::make_shared<xt::xarray<float>>(std::vector<size_t>{num_bonds});
    auto new_vectors = std::make_shared<xt::xarray<Vec3<float>>>(std::vector<size_t>{num_bonds});

    // On shrinking resizes, keep existing data.
    if (num_bonds <= getNumBonds())
    {
        for (unsigned int i = 0; i < num_bonds; i++)
        {
            (*new_neighbors)(i, 0) = (*_neighbors)(i, 0);
            (*new_neighbors)(i, 1) = (*_neighbors)(i, 1);
            (*new_distances)[i] = (*_distances)[i];
            (*new_weights)[i] = (*_weights)[i];
            (*new_vectors)[i] = (*_vectors)[i];
        }
    }

    _neighbors = new_neighbors;
    _distances = new_distances;
    _weights = new_weights;
    _vectors = new_vectors;
    _segments_counts_updated = false;
}

void NeighborList::copy(const NeighborList& other)
{
    _num_query_points = other._num_query_points;
    _num_points = other._num_points;
    _segments_counts_updated = other._segments_counts_updated;
    _neighbors = std::make_shared<xt::xarray<unsigned int>>(*other._neighbors);
    _distances = std::make_shared<xt::xarray<float>>(*other._distances);
    _weights = std::make_shared<xt::xarray<float>>(*other._weights);
    _vectors = std::make_shared<xt::xarray<Vec3<float>>>(*other._vectors);
    _segments = std::make_shared<xt::xarray<unsigned int>>(*other._segments);
    _counts = std::make_shared<xt::xarray<unsigned int>>(*other._counts);
}

void NeighborList::validate(unsigned int num_query_points, unsigned int num_points) const
{
    if (num_query_points != _num_query_points)
    {
        throw std::runtime_error("NeighborList found inconsistent array sizes.");
    }
    if (num_points != _num_points)
    {
        throw std::runtime_error("NeighborList found inconsistent array sizes.");
    }
}

void NeighborList::sort(bool by_distance)
{
    // create a vector of NeighborBonds from the Neighborlist entries
    auto bond_vector = std::move(toBondVector());
    auto num_bonds = bond_vector.size();

    // do single-threaded sort
    if (by_distance)
    {
        std::sort(bond_vector.begin(), bond_vector.end(), compareNeighborDistance);
    }
    else
    {
        std::sort(bond_vector.begin(), bond_vector.end(), compareNeighborBond);
    }

    // put the results back into this neighborlist
    // Simple loop implementation since we don't have the freud utility functions
    for (size_t bond = 0; bond < num_bonds; ++bond)
    {
        auto nb = bond_vector[bond];
        (*_neighbors)(bond, 0) = nb.getQueryPointIdx();
        (*_neighbors)(bond, 1) = nb.getPointIdx();
        (*_distances)(bond) = nb.getDistance();
        (*_vectors)(bond) = nb.getVector();
        (*_weights)(bond) = nb.getWeight();
    }
}

std::vector<NeighborBond> NeighborList::toBondVector() const
{
    auto num_bonds = _distances->size();
    std::vector<NeighborBond> bond_vector(num_bonds);
    // Simple loop implementation since we don't have the freud utility functions
    for (size_t bond_idx = 0; bond_idx < num_bonds; ++bond_idx)
    {
        NeighborBond const nb((*_neighbors)(bond_idx, 0), (*_neighbors)(bond_idx, 1),
                              (*_distances)(bond_idx), (*_weights)(bond_idx), (*_vectors)(bond_idx));
        bond_vector[bond_idx] = nb;
    }
    return bond_vector;
}

unsigned int NeighborList::bisectionSearch(unsigned int val, unsigned int left, unsigned int right) const
{
    if (left + 1 >= right)
    {
        return left;
    }

    unsigned int const middle((left + right) / 2);

    if ((*_neighbors)(middle, 0) < val)
    {
        return bisectionSearch(val, middle, right);
    }
    return bisectionSearch(val, left, middle);
}

bool compareNeighborBond(const NeighborBond& left, const NeighborBond& right)
{
    return left.lessAsTuple(right);
}

bool compareNeighborDistance(const NeighborBond& left, const NeighborBond& right)
{
    return left.lessAsDistance(right);
}

bool compareFirstNeighborPairs(const std::vector<NeighborBond>& left, const std::vector<NeighborBond>& right)
{
    if (right.empty())
    {
        return false;
    }
    if (left.empty())
    {
        return true;
    }
    return compareNeighborBond(left[0], right[0]);
}

}; }; // end namespace molcpp::locality