// Copyright (c) 2010-2025 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef AABB_SIMD_H
#define AABB_SIMD_H

#include "molcpp/locality/AABB.hpp"
#include "xsimd/xsimd.hpp"
#include <iostream> // Added for debug output

/*! \file AABB_simd.h
    \brief SIMD-optimized AABB operations using xsimd
*/

namespace molcpp { namespace locality {

//! SIMD-optimized AABB operations using xsimd
namespace simd {

namespace xs = xsimd;

//! Check if two AABBs overlap using true SIMD operations
/*! \param a First AABB
    \param b Second AABB
    \returns true when the two AABBs overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABB& b)
{
    // Load AABB bounds into SIMD registers
    // We need to handle 3D coordinates, so we'll use the best available architecture
    using batch_type = xs::batch<float, xs::default_arch>;
    
    // Load lower bounds of AABB a - use proper xtensor access
    batch_type a_lower = batch_type::load_unaligned(a.lower.data());
    
    // Load upper bounds of AABB b
    batch_type b_upper = batch_type::load_unaligned(b.upper.data());
    
    // Load upper bounds of AABB a
    batch_type a_upper = batch_type::load_unaligned(a.upper.data());
    
    // Load lower bounds of AABB b
    batch_type b_lower = batch_type::load_unaligned(b.lower.data());
    
    // Check overlap conditions using SIMD operations
    // For overlap: b.upper >= a.lower AND b.lower <= a.upper
    auto upper_check = b_upper >= a_lower;  // b.upper >= a.lower
    auto lower_check = b_lower <= a_upper;  // b.lower <= a.upper
    
    // Combine conditions: all dimensions must overlap
    auto overlap_mask = upper_check & lower_check;
    
    // Return true if all dimensions overlap
    // Since we only have 3D coordinates, only check the first 3 elements
    return (overlap_mask.get(0) && overlap_mask.get(1) && overlap_mask.get(2));
}

//! Check if an AABB and AABBSphere overlap using SIMD operations
/*! \param a AABB
    \param b AABBSphere
    \returns true when the AABB and AABBSphere overlap, false otherwise
*/
inline bool overlap(const AABB& a, const AABBSphere& b)
{
    using batch_type = xs::batch<float, xs::default_arch>;
    
    // Load sphere position and AABB bounds into SIMD registers
    batch_type sphere_pos = batch_type::load_unaligned(b.position.data());
    batch_type aabb_lower = batch_type::load_unaligned(a.lower.data());
    batch_type aabb_upper = batch_type::load_unaligned(a.upper.data());
    
    // Calculate closest point on AABB to sphere center using SIMD
    // closest = max(lower, min(upper, sphere_pos))
    auto closest = xs::max(aabb_lower, xs::min(aabb_upper, sphere_pos));
    
    // Calculate squared distance: (closest - sphere_pos)^2
    auto diff = closest - sphere_pos;
    auto diff_sq = diff * diff;
    
    // Sum the squared differences using SIMD reduction
    float dr2 = xs::reduce_add(diff_sq);
    
    // Check if distance is less than radius squared
    return dr2 < b.radius * b.radius;
}

//! Check if one AABB contains another using SIMD operations
/*! \param a First AABB
    \param b Second AABB
    \returns true when b is fully contained within a
*/
inline bool contains(const AABB& a, const AABB& b)
{
    using batch_type = xs::batch<float, xs::default_arch>;
    
    // Load bounds into SIMD registers
    batch_type a_lower = batch_type::load_unaligned(a.lower.data());
    batch_type a_upper = batch_type::load_unaligned(a.upper.data());
    batch_type b_lower = batch_type::load_unaligned(b.lower.data());
    batch_type b_upper = batch_type::load_unaligned(b.upper.data());
    
    // Check containment: b.lower >= a.lower AND b.upper <= a.upper
    auto lower_check = b_lower >= a_lower;  // b.lower >= a.lower
    auto upper_check = b_upper <= a_upper;  // b.upper <= a.upper
    
    // Combine conditions
    auto contains_mask = lower_check & upper_check;
    
    // Return true if all dimensions are contained
    // Since we only have 3D coordinates, only check the first 3 elements
    return (contains_mask.get(0) && contains_mask.get(1) && contains_mask.get(2));
}

//! Merge two AABBs using SIMD operations
/*! \param a First AABB
    \param b Second AABB
    \returns A new AABB that encloses *a* and *b*
*/
inline AABB merge(const AABB& a, const AABB& b)
{
    using batch_type = xs::batch<float, xs::default_arch>;
    
    AABB new_aabb;
    
    // Load bounds into SIMD registers
    batch_type a_lower = batch_type::load_unaligned(a.lower.data());
    batch_type a_upper = batch_type::load_unaligned(a.upper.data());
    batch_type b_lower = batch_type::load_unaligned(b.lower.data());
    batch_type b_upper = batch_type::load_unaligned(b.upper.data());
    
    // Calculate new bounds using SIMD operations
    batch_type new_lower = xs::min(a_lower, b_lower);
    batch_type new_upper = xs::max(a_upper, b_upper);
    
    // Store results back to the AABB
    new_lower.store_unaligned(new_aabb.lower.data());
    new_upper.store_unaligned(new_aabb.upper.data());
    
    return new_aabb;
}

//! Batch overlap test for multiple AABBs using SIMD operations
/*! \param aabbs Array of AABBs to test
    \param query_aabb The AABB to test against
    \param results Output array for overlap results
    \param count Number of AABBs to test
*/
inline void batch_overlap(const AABB* aabbs, const AABB& query_aabb, bool* results, size_t count)
{
    using batch_type = xs::batch<float, xs::default_arch>;
    constexpr size_t batch_size = batch_type::size;
    
    // Load query AABB bounds once into SIMD registers
    batch_type query_lower = batch_type::load_unaligned(query_aabb.lower.data());
    batch_type query_upper = batch_type::load_unaligned(query_aabb.upper.data());
    
    // Process AABBs in SIMD batch sizes
    size_t i = 0;
    for (; i + batch_size <= count; i += batch_size)
    {
        // Process batch of AABBs using SIMD
        for (size_t j = 0; j < batch_size; ++j)
        {
            const AABB& current = aabbs[i + j];
            batch_type current_lower = batch_type::load_unaligned(current.lower.data());
            batch_type current_upper = batch_type::load_unaligned(current.upper.data());
            
            auto upper_check = current_upper >= query_lower;
            auto lower_check = current_lower <= query_upper;
            auto overlap_mask = upper_check & lower_check;
            // Since we only have 3D coordinates, only check the first 3 elements
            results[i + j] = (overlap_mask.get(0) && overlap_mask.get(1) && overlap_mask.get(2));
        }
    }
    
    // Handle remaining AABBs using regular overlap function
    for (; i < count; ++i)
    {
        // Use the regular overlap function from AABB.hpp to avoid ambiguity
        const AABB& current = aabbs[i];
        results[i] = (current.upper[0] >= query_aabb.lower[0] && current.lower[0] <= query_aabb.upper[0] &&
                      current.upper[1] >= query_aabb.lower[1] && current.lower[1] <= query_aabb.upper[1] &&
                      current.upper[2] >= query_aabb.lower[2] && current.lower[2] <= query_aabb.upper[2]);
    }
}

} // namespace simd

} // namespace locality
} // namespace molcpp

#endif // AABB_SIMD_H
