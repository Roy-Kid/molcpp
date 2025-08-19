// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef AABB_H
#define AABB_H

#include "molcpp/types.hpp"

/*! \file AABB.h
    \brief Basic AABB routines
*/

#if defined _WIN32
#define CACHE_ALIGN __declspec(align(32))
#else
#define CACHE_ALIGN __attribute__((aligned(32)))
#endif

namespace molcpp { namespace locality {

//! Axis aligned bounding box
/*! An AABB represents a bounding volume defined by an axis-aligned bounding box. It is stored as plain old
   data with a lower and upper bound. This is to make the most common operation of AABB overlap testing fast.

    Do not access data members directly. AABB uses SSE and AVX optimizations and the internal data format
   changes. It also changes between the CPU and GPU. Instead, use the accessor methods getLower(), getUpper()
   and getPosition().

    Operations are provided as free functions to perform the following operations:

    - merge()
    - overlap()
    - contains()
*/
struct CACHE_ALIGN AABB
{
    Vec3<float> lower; //!< Lower left corner
    Vec3<float> upper; //!< Upper right corner

    unsigned int tag; //!< Optional tag id, useful for particle ids

    //! Default construct a 0 AABB
    AABB() : lower(Vec3<float>{0.0f, 0.0f, 0.0f}), upper(Vec3<float>{0.0f, 0.0f, 0.0f}), tag(0)
    {
        // Explicitly initialize Vec3 members to zero
    }

    //! Construct an AABB from the given lower and upper corners
    /*! \param _lower Lower left corner of the AABB
        \param _upper Upper right corner of the AABB
    */
    AABB(const Vec3<float>& _lower, const Vec3<float>& _upper) : tag(0)
    {
        lower = _lower;
        upper = _upper;
    }

    //! Construct an AABB from a sphere
    /*! \param _position Position of the sphere
        \param radius Radius of the sphere
    */
    AABB(const Vec3<float>& _position, float radius) : tag(0)
    {
        Vec3<float> new_lower;
        Vec3<float> new_upper;
        new_lower[0] = _position[0] - radius;
        new_lower[1] = _position[1] - radius;
        new_lower[2] = _position[2] - radius;
        new_upper[0] = _position[0] + radius;
        new_upper[1] = _position[1] + radius;
        new_upper[2] = _position[2] + radius;

        lower = new_lower;
        upper = new_upper;
    }

    //! Construct an AABB from a point with a particle tag
    /*! \param _position Position of the point
        \param _tag Global particle tag id
    */
    AABB(const Vec3<float>& _position, unsigned int _tag) : tag(_tag)
    {
        lower = _position;
        upper = _position;
    }

    //! Get the AABB's position
    Vec3<float> getPosition() const
    {
        return (lower + upper) / float(2);
    }

    //! Get the AABB's lower point
    Vec3<float> getLower() const
    {
        return lower;
    }

    //! Get the AABB's upper point
    Vec3<float> getUpper() const
    {
        return upper;
    }

    //! Translate the AABB by the given vector
    void translate(const Vec3<float>& v)
    {
        upper += v;
        lower += v;
    }
};

struct CACHE_ALIGN AABBSphere
{
    Vec3<float> position; //!< Sphere position

    float radius;     //!< Radius of sphere
    unsigned int tag; //!< Optional tag id, useful for particle ids

    //! Default construct a 0 AABBSphere
    AABBSphere() : position(Vec3<float>{0.0f, 0.0f, 0.0f}), radius(0), tag(0)
    {
        // Explicitly initialize Vec3 members to zero
    }

    //! Construct an AABBSphere from the given position and radius
    /*! \param _position Position of the sphere
        \param _radius Radius of the sphere
    */
    AABBSphere(const Vec3<float>& _position, float _radius) : radius(_radius), tag(0)
    {
        position = _position;
    }

    //! Construct an AABBSphere from the given position and radius with a tag
    /*! \param _position Position of the sphere
        \param _radius Radius of the sphere
        \param _tag Global particle tag id
    */
    AABBSphere(const Vec3<float>& _position, float _radius, unsigned int _tag) : radius(_radius), tag(_tag)
    {
        position = _position;
    }

    //! Get the AABBSphere's position
    Vec3<float> getPosition() const
    {
        return position;
    }

    //! Translate the AABBSphere by the given vector
    void translate(const Vec3<float>& v)
    {
        position += v;
    }
};

//! Check if two AABBs overlap
/*! \param a First AABB
    \param b Second AABB
    \returns true when the two AABBs overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABB& b)
{
    return b.upper[0] >= a.lower[0] && b.lower[0] <= a.upper[0] && b.upper[1] >= a.lower[1]
        && b.lower[1] <= a.upper[1] && b.upper[2] >= a.lower[2] && b.lower[2] <= a.upper[2];
}

//! Check if an AABB and AABBSphere overlap
/*! \param a AABB
    \param b AABBSphere
    \returns true when the AABB and AABBSphere overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABBSphere& b)
{
    Vec3<float> const dr = Vec3<float>{std::min(std::max(b.position[0], a.lower[0]), a.upper[0]) - b.position[0],
                                       std::min(std::max(b.position[1], a.lower[1]), a.upper[1]) - b.position[1],
                                       std::min(std::max(b.position[2], a.lower[2]), a.upper[2]) - b.position[2]};
    float const dr2 = xt::sum(dr * dr)();
    return dr2 < b.radius * b.radius;
}

//! Check if one AABB contains another
/*! \param a First AABB
    \param b Second AABB
    \returns true when b is fully contained within a
*/
inline bool contains(const AABB& a, const AABB& b)
{
    return (b.lower[0] >= a.lower[0] && b.upper[0] <= a.upper[0] && b.lower[1] >= a.lower[1]
            && b.upper[1] <= a.upper[1] && b.lower[2] >= a.lower[2] && b.upper[2] <= a.upper[2]);
}

//! Merge two AABBs
/*! \param a First AABB
    \param b Second AABB
    \returns A new AABB that encloses *a* and *b*
*/
inline AABB merge(const AABB& a, const AABB& b)
{
    AABB new_aabb;
    new_aabb.lower[0] = std::min(a.lower[0], b.lower[0]);
    new_aabb.lower[1] = std::min(a.lower[1], b.lower[1]);
    new_aabb.lower[2] = std::min(a.lower[2], b.lower[2]);
    new_aabb.upper[0] = std::max(a.upper[0], b.upper[0]);
    new_aabb.upper[1] = std::max(a.upper[1], b.upper[1]);
    new_aabb.upper[2] = std::max(a.upper[2], b.upper[2]);

    return new_aabb;
}

}; }; // end namespace molcpp::locality

#endif // AABB_H