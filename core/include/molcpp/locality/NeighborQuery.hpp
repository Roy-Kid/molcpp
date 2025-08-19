// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef NEIGHBOR_QUERY_H
#define NEIGHBOR_QUERY_H

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "molcpp/spatial/box.hpp"
#include "molcpp/locality/NeighborBond.hpp"
#include "molcpp/locality/NeighborList.hpp"
#include "molcpp/locality/NeighborPerPointIterator.hpp"
#include "molcpp/types.hpp"

/*! \file NeighborQuery.h
    \brief Defines the abstract API for collections of points that can be
           queried against for neighbors.
*/

namespace molcpp { namespace locality {

//! Enumeration for types of queries.
enum class QueryType
{
    none,    //! Default query type to avoid implicit default types.
    ball,    //! Query based on distance cutoff.
    nearest, //! Query based on number of requested neighbors.
};

constexpr auto DEFAULT_MODE = QueryType::none;            //!< Default mode.
constexpr unsigned int DEFAULT_NUM_NEIGHBORS(0xffffffff); //!< Default number of neighbors.
constexpr float DEFAULT_R_MAX(-1.0);                      //!< Default maximum query distance.
constexpr float DEFAULT_R_MIN(0);                         //!< Default minimum query distance.
constexpr float DEFAULT_R_GUESS(-1.0);                    //!< Default guess query distance.
constexpr float DEFAULT_SCALE(-1.0);      //!< Default scaling parameter for AABB nearest neighbor queries.
constexpr bool DEFAULT_EXCLUDE_II(false); //!< Default for whether or not to include self-neighbors.

//! POD class to hold information about generic queries.
/*! This class provides a standard method for specifying the type of query to
 *  perform with a NeighborQuery object. Rather than calling queryBall
 *  specifically, for example, the user can call a generic querying function and
 *  provide an instance of this class to specify the nature of the query.
 */
struct QueryArgs
{
    QueryArgs() = default;

    QueryType mode {DEFAULT_MODE}; //! Whether to perform a ball or k-nearest neighbor query.
    unsigned int num_neighbors {DEFAULT_NUM_NEIGHBORS}; //! The number of nearest neighbors to find.
    float r_max {DEFAULT_R_MAX};          //! The cutoff distance within which to find neighbors.
    float r_min {DEFAULT_R_MIN};          //! The minimum distance beyond which to find neighbors.
    float r_guess {DEFAULT_R_GUESS};      //! The initial distance for finding neighbors, used by some
                                          //! algorithms to initialize a number of neighbors query.
    float scale {DEFAULT_SCALE};          //! The scale factor to use when performing repeated ball queries
                                          //! to find a specified number of nearest neighbors.
    bool exclude_ii {DEFAULT_EXCLUDE_II}; //! If true, exclude self-neighbors.
};

// Forward declare the iterators
class NeighborQueryIterator;
class NeighborQueryPerPointIterator;

//! Parent data structure for all neighbor finding algorithms.
/*! This class defines the API for all data structures for accelerating
 *  neighbor finding. The object encapsulates a set of points and a system box
 *  that define the set of points to search and the periodic system within these
 *  points can be found. The interface for finding neighbors is the query
 *  method, which generates an iterator that finds all requested neighbors.
 */
class NeighborQuery
{
public:
    //! Nullary constructor for Cython
    NeighborQuery() = default;

    //! Constructor
    NeighborQuery(const Box& box, const XYZ& points)
        : m_box(box), m_points(points), m_n_points(points.shape(0))
    {
    }

    //! Destructor
    virtual ~NeighborQuery() = default;

    //! Query for neighbors of multiple points
    /*! \param query_points The points to find neighbors for.
     *  \param query_args The query arguments that should be used to find neighbors.
     */
    virtual std::shared_ptr<NeighborQueryIterator>
    query(const XYZ& query_points, QueryArgs query_args) const
    {
        // pair calculations using non-periodic boxes should fail
        Vec3<bool> const periodic = m_box.pbc();
        if (!(periodic[0] && periodic[1] && periodic[2]))
        {
            throw std::runtime_error("NeighborQuery requires periodic boundary conditions.");
        }

        return std::make_shared<NeighborQueryIterator>(this, query_points, query_points.shape(0), query_args);
    }

    //! Query for neighbors of a single point
    /*! \param query_point The point to find neighbors for.
     *  \param query_point_idx The index of the query point.
     *  \param args The query arguments that should be used to find neighbors.
     */
    virtual std::shared_ptr<NeighborQueryPerPointIterator>
    querySingle(const Vec3<float> query_point, unsigned int query_point_idx, QueryArgs args) const = 0;

    //! Get the simulation box
    const Box& getBox() const
    {
        return m_box;
    }

    //! Get the number of reference points
    unsigned int getNPoints() const
    {
        return m_n_points;
    }

    //! Get the reference points
    const XYZ& getPoints() const
    {
        return m_points;
    }

    //! Access a specific point by index
    /*! \param index The point index to return.
     */
    Vec3<float> operator[](unsigned int index) const
    {
        if (index >= m_n_points)
        {
            throw std::out_of_range("Point index out of range.");
        }
        Vec3<float> point;
        point[0] = m_points(index, 0);
        point[1] = m_points(index, 1);
        point[2] = m_points(index, 2);
        return point;
    }

protected:
    //! Validate the combination of specified arguments.
    virtual void validateQueryArgs(QueryArgs& args) const
    {
        if (args.mode == QueryType::none)
        {
            args.mode = DEFAULT_MODE;
        }
        if (args.num_neighbors == DEFAULT_NUM_NEIGHBORS)
        {
            args.num_neighbors = 1;
        }
        if (args.r_max == DEFAULT_R_MAX)
        {
            args.r_max = std::numeric_limits<float>::max();
        }
        if (args.r_min == DEFAULT_R_MIN)
        {
            args.r_min = 0;
        }
        if (args.r_guess == DEFAULT_R_GUESS)
        {
            args.r_guess = args.r_max;
        }
        if (args.scale == DEFAULT_SCALE)
        {
            args.scale = 1.0f;
        }
        if (args.exclude_ii == DEFAULT_EXCLUDE_II)
        {
            args.exclude_ii = false;
        }
    }

    const Box& m_box;        //!< Simulation box where the particles belong.
    const XYZ m_points; //!< Point coordinates.
    unsigned int m_n_points;     //!< Number of points.
};

//! Implementation of per-point finding logic for NeighborQuery objects.
/*! This abstract class specializes a few of the methods of its parent for the
 *  case of working with NeighborQuery objects. In particular, on construction
 *  it takes information on the NeighborQuery object it is attached to, allowing
 *  iterators to access the points the NeighborQuery was constructed for.
 *  Additionally, it defines the standard interface by which such iterations
 *  should indicate that all neighbors have been found, which is by setting the
 *  m_finished flag.
 */
class NeighborQueryPerPointIterator : public NeighborPerPointIterator
{
public:
    //! Nullary constructor for Cython
    NeighborQueryPerPointIterator() = default;

    //! Constructor
    NeighborQueryPerPointIterator(const NeighborQuery* neighbor_query, const Vec3<float>& query_point,
                                  unsigned int query_point_idx, float r_max, float r_min, bool exclude_ii)
        : NeighborPerPointIterator(query_point_idx), m_neighbor_query(neighbor_query),
          m_query_point(query_point), m_finished(false), m_r_max(r_max), m_r_min(r_min),
          m_exclude_ii(exclude_ii)
    {
        if (r_max <= 0)
        {
            throw std::invalid_argument("NeighborQuery requires r_max to be positive.");
        }
        if (r_max <= r_min)
        {
            throw std::invalid_argument("NeighborQuery requires that r_max must be greater than r_min.");
        }
    }

    //! Empty Destructor
    ~NeighborQueryPerPointIterator() override = default;

    //! Indicate when done.
    bool end() const override
    {
        return m_finished;
    }

    //! Get the next element.
    NeighborBond next() override = 0;

protected:
    const NeighborQuery* m_neighbor_query;       //!< Link to the NeighborQuery object.
    const Vec3<float> m_query_point = {0, 0, 0}; //!< Coordinates of the query point.
    bool m_finished; //!< Flag to indicate that iteration is complete (must be set by next() on termination).
    float m_r_max;   //!< Cutoff distance for neighbors.
    float m_r_min;   //!< Minimum distance for neighbors.
    bool m_exclude_ii; //!< Flag to indicate whether or not to include self bonds.
};

//! The iterator class for neighbor queries on NeighborQuery objects.
/*! All queries to a NeighborQuery return instances of this class. The
 *  NeighborQueryIterator is capable of either iterating over all neighbors of
 *  the provided query_points based on the set of points contained in the
 *  NeighborQuery object, or of providing NeighborQueryPerPoint iterator
 *  instances for any of the query_points it was constructed with. The first
 *  interface is much more convenient for user interaction, while the second is
 *  primarily provided to support thread-safe parallelism. This class is not
 *  designed to be inherited from; the implementation of the querySingle method
 *  in NeighborQuery subclasses (to return per-point iterators) should be
 *  sufficient for this class to work.
 */
class NeighborQueryIterator
{
public:
    //! Constructor
    NeighborQueryIterator(const NeighborQuery* neighbor_query, const XYZ& query_points,
                          unsigned int num_query_points, const QueryArgs& qargs)
        : m_neighbor_query(neighbor_query), m_query_points(query_points),
          m_num_query_points(num_query_points), m_qargs(qargs)
    {
        m_iter = this->query(m_cur_p);
    }

    //! Empty Destructor
    ~NeighborQueryIterator() = default;

    //! Indicate when done.
    bool end() const
    {
        return m_finished;
    }

    //! Get an iterator for a specific query point by index.
    std::shared_ptr<NeighborQueryPerPointIterator> query(unsigned int i)
    {
        Vec3<float> point;
        point[0] = m_query_points(i, 0);
        point[1] = m_query_points(i, 1);
        point[2] = m_query_points(i, 2);
        return m_neighbor_query->querySingle(point, i, m_qargs);
    }

    //! Get the next element.
    NeighborBond next()
    {
        if (m_finished)
        {
            return NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f}); //!< The object returned when iteration is complete.
        }
        NeighborBond nb;
        while (true)
        {
            while (!m_iter->end())
            {
                nb = m_iter->next();

                if (nb != NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f}))
                {
                    return nb;
                }
            }
            m_cur_p++;
            if (m_cur_p >= m_num_query_points)
            {
                break;
            }
            m_iter = this->query(m_cur_p);
        }
        m_finished = true;
        return NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f}); //!< The object returned when iteration is complete.
    }

    //! Generate a NeighborList from query.
    /*! This function exploits parallelism by finding the neighbors for
     *  each query point in parallel and adding them to a list, which is
     *  then sorted in parallel as well before being added to the
     *  NeighborList object. Right now this won't be backwards compatible
     *  because the kn query is not symmetric, so even if we reverse the
     *  output order here the actual neighbors found will be different.
     *
     *  This function returns a pointer, not a shared pointer, so the
     *  caller is responsible for deleting it. The reason for this is that
     *  the primary use-case is to have this object be managed by instances
     *  of the Cython NeighborList class.
     */
    std::shared_ptr<NeighborList> toNeighborList(bool sort_by_distance = false)
    {
        using BondVector = std::vector<NeighborBond>;
        BondVector bonds;
        for (size_t i = 0; i < m_num_query_points; ++i)
        {
            std::shared_ptr<NeighborQueryPerPointIterator> const it = this->query(i);
            while (!it->end())
            {
                NeighborBond nb = it->next();
                // If we're excluding ii bonds, we have to check before adding.
                if (nb != NeighborBond(-1, -1, 0, 0, Vec3<float>{0.0f, 0.0f, 0.0f}))
                {
                    bonds.emplace_back(nb.getQueryPointIdx(), nb.getPointIdx(), nb.getWeight(),
                                             nb.getVector());
                }
            }
        }

        std::vector<NeighborBond> linear_bonds(bonds.begin(), bonds.end());
        if (sort_by_distance)
        {
            std::sort(linear_bonds.begin(), linear_bonds.end(), compareNeighborDistance);
        }
        else
        {
            std::sort(linear_bonds.begin(), linear_bonds.end(), compareNeighborBond);
        }

        unsigned int const num_bonds = linear_bonds.size();

        auto nl = std::make_shared<NeighborList>();
        nl->setNumBonds(num_bonds, m_num_query_points, m_neighbor_query->getNPoints());

        for (size_t bond = 0; bond < num_bonds; ++bond)
        {
            nl->setNeighborEntry(bond, linear_bonds[bond]);
        }

        return nl;
    }

protected:
    const NeighborQuery* m_neighbor_query;                 //!< Link to the NeighborQuery object.
    const XYZ m_query_points;                     //!< Coordinates of the query points.
    unsigned int m_num_query_points;                       //!< The number of query points.
    const QueryArgs m_qargs;                               //!< The query arguments
    std::shared_ptr<NeighborQueryPerPointIterator> m_iter; //!< The per-point iterator being used.

    bool m_finished {
        false}; //!< Flag to indicate that iteration is complete (must be set by next on termination).
    unsigned int m_cur_p {0}; //!< The current particle under consideration.
};

}; }; // end namespace freud::locality

#endif // NEIGHBOR_QUERY_H