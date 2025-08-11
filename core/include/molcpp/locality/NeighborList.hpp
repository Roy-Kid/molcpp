#ifndef MOLCPP_LOCALITY_NEIGHBORLIST_HPP
#define MOLCPP_LOCALITY_NEIGHBORLIST_HPP

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <xtensor/xarray.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xview.hpp>

#include "NeighborBond.hpp"
#include "../types.hpp"

namespace molcpp {
namespace locality {

//! Class for storing and managing neighbor lists
/*! The NeighborList class stores a list of neighbor bonds/pairs and provides
    methods for filtering, sorting, and accessing the neighbor information.
*/
class NeighborList {
public:
    //! Default constructor
    NeighborList() : m_sorted(false) {}

    //! Constructor from vector of bonds
    explicit NeighborList(const std::vector<NeighborBond>& bonds)
        : m_bonds(bonds), m_sorted(false) {}

    //! Constructor with capacity reservation
    explicit NeighborList(size_t capacity) : m_sorted(false) {
        m_bonds.reserve(capacity);
    }

    //! Add a neighbor bond
    void addBond(const NeighborBond& bond) {
        m_bonds.push_back(bond);
        m_sorted = false;
    }

    //! Add a neighbor bond with basic parameters
    void addBond(unsigned int query_idx, unsigned int point_idx, double distance) {
        m_bonds.emplace_back(query_idx, point_idx, distance);
        m_sorted = false;
    }

    //! Add a neighbor bond with all parameters
    void addBond(unsigned int query_idx, unsigned int point_idx, 
                 double distance, double weight, const Vec3& distance_vec) {
        m_bonds.emplace_back(query_idx, point_idx, distance, weight, distance_vec);
        m_sorted = false;
    }

    //! Get the number of bonds
    size_t size() const { return m_bonds.size(); }

    //! Check if the list is empty
    bool empty() const { return m_bonds.empty(); }

    //! Clear all bonds
    void clear() {
        m_bonds.clear();
        m_sorted = false;
    }

    //! Reserve capacity
    void reserve(size_t capacity) {
        m_bonds.reserve(capacity);
    }

    //! Sort the neighbor list by query point index, then point index
    void sort() {
        std::sort(m_bonds.begin(), m_bonds.end());
        m_sorted = true;
    }

    //! Check if the list is sorted
    bool isSorted() const { return m_sorted; }

    //! Get a bond by index
    const NeighborBond& operator[](size_t idx) const {
        if (idx >= m_bonds.size()) {
            throw std::out_of_range("Bond index out of range");
        }
        return m_bonds[idx];
    }

    //! Get a bond by index (non-const)
    NeighborBond& operator[](size_t idx) {
        if (idx >= m_bonds.size()) {
            throw std::out_of_range("Bond index out of range");
        }
        m_sorted = false;  // Assume modification breaks sorting
        return m_bonds[idx];
    }

    //! Get all bonds
    const std::vector<NeighborBond>& getBonds() const { return m_bonds; }

    //! Get query point indices as xtensor array
    xt::xarray<unsigned int> getQueryPointIndices() const {
        std::vector<unsigned int> indices;
        indices.reserve(m_bonds.size());
        for (const auto& bond : m_bonds) {
            indices.push_back(bond.query_point_idx);
        }
        return xt::adapt(indices, {indices.size()});
    }

    //! Get point indices as xtensor array
    xt::xarray<unsigned int> getPointIndices() const {
        std::vector<unsigned int> indices;
        indices.reserve(m_bonds.size());
        for (const auto& bond : m_bonds) {
            indices.push_back(bond.point_idx);
        }
        return xt::adapt(indices, {indices.size()});
    }

    //! Get distances as xtensor array
    xt::xarray<double> getDistances() const {
        std::vector<double> distances;
        distances.reserve(m_bonds.size());
        for (const auto& bond : m_bonds) {
            distances.push_back(bond.distance);
        }
        return xt::adapt(distances, {distances.size()});
    }

    //! Get weights as xtensor array
    xt::xarray<double> getWeights() const {
        std::vector<double> weights;
        weights.reserve(m_bonds.size());
        for (const auto& bond : m_bonds) {
            weights.push_back(bond.weight);
        }
        return xt::adapt(weights, {weights.size()});
    }

    //! Get distance vectors as xtensor array
    xt::xarray<double> getDistanceVectors() const {
        std::vector<double> vectors;
        vectors.reserve(m_bonds.size() * 3);
        for (const auto& bond : m_bonds) {
            vectors.push_back(bond.distance_vec(0));
            vectors.push_back(bond.distance_vec(1));
            vectors.push_back(bond.distance_vec(2));
        }
        return xt::adapt(vectors, {m_bonds.size(), size_t(3)});
    }

    //! Filter bonds by distance range
    /*! \param r_max Maximum distance (inclusive)
        \param r_min Minimum distance (inclusive)
    */
    void filterR(double r_max, double r_min = 0.0) {
        if (r_min < 0 || r_max < 0) {
            throw std::invalid_argument("Distance limits must be non-negative");
        }
        if (r_min > r_max) {
            throw std::invalid_argument("r_min must be less than or equal to r_max");
        }

        auto new_end = std::remove_if(m_bonds.begin(), m_bonds.end(),
            [r_min, r_max](const NeighborBond& bond) {
                return bond.distance < r_min || bond.distance > r_max;
            });
        m_bonds.erase(new_end, m_bonds.end());
    }

    //! Get neighbors for a specific query point
    /*! \param query_idx Query point index
        \returns Vector of bonds for the query point
    */
    std::vector<NeighborBond> getNeighborsForPoint(unsigned int query_idx) const {
        std::vector<NeighborBond> neighbors;
        for (const auto& bond : m_bonds) {
            if (bond.query_point_idx == query_idx) {
                neighbors.push_back(bond);
            }
        }
        return neighbors;
    }

    //! Get the number of neighbors for each query point
    /*! \param num_query_points Total number of query points
        \returns Array of neighbor counts
    */
    xt::xarray<unsigned int> getNeighborCounts(unsigned int num_query_points) const {
        xt::xarray<unsigned int> counts = xt::zeros<unsigned int>({num_query_points});
        for (const auto& bond : m_bonds) {
            if (bond.query_point_idx < num_query_points) {
                counts(bond.query_point_idx)++;
            }
        }
        return counts;
    }

    //! Iterator support
    using iterator = std::vector<NeighborBond>::iterator;
    using const_iterator = std::vector<NeighborBond>::const_iterator;

    iterator begin() { m_sorted = false; return m_bonds.begin(); }
    iterator end() { m_sorted = false; return m_bonds.end(); }
    const_iterator begin() const { return m_bonds.begin(); }
    const_iterator end() const { return m_bonds.end(); }
    const_iterator cbegin() const { return m_bonds.cbegin(); }
    const_iterator cend() const { return m_bonds.cend(); }

private:
    std::vector<NeighborBond> m_bonds;  //!< Storage for neighbor bonds
    bool m_sorted;                       //!< Whether the list is sorted
};

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_NEIGHBORLIST_HPP