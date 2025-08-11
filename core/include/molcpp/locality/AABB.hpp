#ifndef MOLCPP_LOCALITY_AABB_HPP
#define MOLCPP_LOCALITY_AABB_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <xtensor/xarray.hpp>
#include <xtensor/xfixed.hpp>
#include <xtensor/xview.hpp>

#ifdef XTENSOR_USE_XSIMD
#include <xsimd/xsimd.hpp>
#endif

#include "../types.hpp"

namespace molcpp {
namespace locality {

// Cache alignment for performance
#if defined _WIN32
#define CACHE_ALIGN __declspec(align(32))
#else
#define CACHE_ALIGN __attribute__((aligned(32)))
#endif

//! Axis aligned bounding box
/*! An AABB represents a bounding volume defined by an axis-aligned bounding box.
    It is stored as plain old data with a lower and upper bound.
    This is to make the most common operation of AABB overlap testing fast.
*/
struct CACHE_ALIGN AABB {
    Vec3 lower; //!< Lower left corner
    Vec3 upper; //!< Upper right corner
    unsigned int tag; //!< Optional tag id, useful for particle ids

    //! Default construct a 0 AABB
    AABB() : tag(0) {
        lower.fill(0.0);
        upper.fill(0.0);
    }

    //! Construct an AABB from the given lower and upper corners
    /*! \param _lower Lower left corner of the AABB
        \param _upper Upper right corner of the AABB
    */
    AABB(const Vec3& _lower, const Vec3& _upper) 
        : lower(_lower), upper(_upper), tag(0) {}

    //! Construct an AABB from a sphere
    /*! \param _position Position of the sphere
        \param radius Radius of the sphere
    */
    AABB(const Vec3& _position, double radius) : tag(0) {
        for (size_t i = 0; i < 3; ++i) {
            lower(i) = _position(i) - radius;
            upper(i) = _position(i) + radius;
        }
    }

    //! Construct an AABB from a point with a particle tag
    /*! \param _position Position of the point
        \param _tag Global particle tag id
    */
    AABB(const Vec3& _position, unsigned int _tag) 
        : lower(_position), upper(_position), tag(_tag) {}

    //! Get the AABB's position (center)
    Vec3 getPosition() const {
        Vec3 result;
        for (size_t i = 0; i < 3; ++i) {
            result(i) = (lower(i) + upper(i)) * 0.5;
        }
        return result;
    }

    //! Get the AABB's lower point
    const Vec3& getLower() const { return lower; }

    //! Get the AABB's upper point
    const Vec3& getUpper() const { return upper; }

    //! Translate the AABB by the given vector
    void translate(const Vec3& v) {
        lower += v;
        upper += v;
    }
};

//! Sphere representation for AABB operations
struct CACHE_ALIGN AABBSphere {
    Vec3 position; //!< Sphere position
    double radius; //!< Radius of sphere
    unsigned int tag; //!< Optional tag id, useful for particle ids

    //! Default construct a 0 AABBSphere
    AABBSphere() : radius(0), tag(0) {
        position.fill(0.0);
    }

    //! Construct an AABBSphere from the given position and radius
    /*! \param _position Position of the sphere
        \param _radius Radius of the sphere
    */
    AABBSphere(const Vec3& _position, double _radius) 
        : position(_position), radius(_radius), tag(0) {}

    //! Construct an AABBSphere from the given position and radius with a tag
    /*! \param _position Position of the sphere
        \param _radius Radius of the sphere
        \param _tag Global particle tag id
    */
    AABBSphere(const Vec3& _position, double _radius, unsigned int _tag) 
        : position(_position), radius(_radius), tag(_tag) {}

    //! Get the AABBSphere's position
    const Vec3& getPosition() const { return position; }

    //! Translate the AABBSphere by the given vector
    void translate(const Vec3& v) {
        position += v;
    }
};

//! Check if two AABBs overlap
/*! \param a First AABB
    \param b Second AABB
    \returns true when the two AABBs overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABB& b) {
#ifdef XTENSOR_USE_XSIMD
    using batch_type = xsimd::batch<double>;
    
    // Load lower and upper bounds
    auto a_lower = batch_type::load_aligned(a.lower.data());
    auto a_upper = batch_type::load_aligned(a.upper.data());
    auto b_lower = batch_type::load_aligned(b.lower.data());
    auto b_upper = batch_type::load_aligned(b.upper.data());
    
    // Check overlap conditions
    auto lower_check = b_upper >= a_lower;
    auto upper_check = b_lower <= a_upper;
    
    // All dimensions must overlap
    return xsimd::all(lower_check & upper_check);
#else
    return b.upper(0) >= a.lower(0) && b.lower(0) <= a.upper(0) &&
           b.upper(1) >= a.lower(1) && b.lower(1) <= a.upper(1) &&
           b.upper(2) >= a.lower(2) && b.lower(2) <= a.upper(2);
#endif
}

//! Check if an AABB and AABBSphere overlap
/*! \param a AABB
    \param b AABBSphere
    \returns true when the AABB and AABBSphere overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABBSphere& b) {
    Vec3 dr;
    for (size_t i = 0; i < 3; ++i) {
        dr(i) = std::min(std::max(b.position(i), a.lower(i)), a.upper(i)) - b.position(i);
    }
    
    double dr2 = 0.0;
    for (size_t i = 0; i < 3; ++i) {
        dr2 += dr(i) * dr(i);
    }
    
    return dr2 < b.radius * b.radius;
}

//! Check if one AABB contains another
/*! \param a First AABB
    \param b Second AABB
    \returns true when b is fully contained within a
*/
inline bool contains(const AABB& a, const AABB& b) {
#ifdef XTENSOR_USE_XSIMD
    using batch_type = xsimd::batch<double>;
    
    auto a_lower = batch_type::load_aligned(a.lower.data());
    auto a_upper = batch_type::load_aligned(a.upper.data());
    auto b_lower = batch_type::load_aligned(b.lower.data());
    auto b_upper = batch_type::load_aligned(b.upper.data());
    
    auto lower_check = b_lower >= a_lower;
    auto upper_check = b_upper <= a_upper;
    
    return xsimd::all(lower_check & upper_check);
#else
    return b.lower(0) >= a.lower(0) && b.upper(0) <= a.upper(0) &&
           b.lower(1) >= a.lower(1) && b.upper(1) <= a.upper(1) &&
           b.lower(2) >= a.lower(2) && b.upper(2) <= a.upper(2);
#endif
}

//! Merge two AABBs
/*! \param a First AABB
    \param b Second AABB
    \returns A new AABB that encloses *a* and *b*
*/
inline AABB merge(const AABB& a, const AABB& b) {
    AABB new_aabb;
    
#ifdef XTENSOR_USE_XSIMD
    using batch_type = xsimd::batch<double>;
    
    auto a_lower = batch_type::load_aligned(a.lower.data());
    auto a_upper = batch_type::load_aligned(a.upper.data());
    auto b_lower = batch_type::load_aligned(b.lower.data());
    auto b_upper = batch_type::load_aligned(b.upper.data());
    
    auto min_lower = xsimd::min(a_lower, b_lower);
    auto max_upper = xsimd::max(a_upper, b_upper);
    
    min_lower.store_aligned(new_aabb.lower.data());
    max_upper.store_aligned(new_aabb.upper.data());
#else
    for (size_t i = 0; i < 3; ++i) {
        new_aabb.lower(i) = std::min(a.lower(i), b.lower(i));
        new_aabb.upper(i) = std::max(a.upper(i), b.upper(i));
    }
#endif
    
    return new_aabb;
}

} // namespace locality
} // namespace molcpp

#endif // MOLCPP_LOCALITY_AABB_HPP