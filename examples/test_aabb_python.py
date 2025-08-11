#!/usr/bin/env python3
"""
Example demonstrating AABB neighbor finding using molcpp Python bindings.
"""

import numpy as np
import time

# This would be imported from the compiled module
# import molcpp_locality

def demo_aabb_neighbor_finding():
    """
    Demonstrate AABB neighbor finding functionality.
    
    Note: This is a demonstration of how the Python API would work.
    The actual module needs to be compiled first.
    """
    
    print("AABB Neighbor Finding Python Example")
    print("=" * 40)
    
    # Create a cubic box
    L = 10.0
    # box = molcpp_locality.Box(L, L, L)
    print(f"Created box with dimensions: {L} x {L} x {L}")
    
    # Generate random points
    N = 1000
    points = np.random.rand(N, 3) * L - L/2.0
    print(f"Generated {N} random points\n")
    
    # Create AABB query structure
    # query = molcpp_locality.AABBQuery(box, points)
    
    # Example 1: Ball query
    print("Example 1: Ball Query")
    print("-" * 20)
    r_max = 1.0
    # args = molcpp_locality.QueryArgs.ball(r_max, 0.0, exclude_ii=True)
    
    # start = time.time()
    # nlist = query.query(points, args)
    # elapsed = time.time() - start
    
    # print(f"Found {len(nlist)} neighbor pairs within radius {r_max}")
    # print(f"Query time: {elapsed*1000:.2f} milliseconds")
    
    # Calculate average neighbors per particle
    # counts = nlist.get_neighbor_counts(N)
    # avg_neighbors = np.mean(counts)
    # print(f"Average neighbors per particle: {avg_neighbors:.2f}\n")
    
    # Example 2: K-nearest neighbors
    print("Example 2: K-Nearest Neighbors Query")
    print("-" * 35)
    k = 6
    # args = molcpp_locality.QueryArgs.nearest(k, exclude_ii=True)
    
    # Query only first 100 points
    query_points = points[:100]
    
    # start = time.time()
    # nlist = query.query(query_points, args)
    # elapsed = time.time() - start
    
    # print(f"Found {k} nearest neighbors for 100 query points")
    # print(f"Total neighbor pairs: {len(nlist)}")
    # print(f"Query time: {elapsed*1000:.2f} milliseconds\n")
    
    # Example 3: Range query
    print("Example 3: Range Query")
    print("-" * 20)
    r_min = 0.5
    r_max = 1.5
    # args = molcpp_locality.QueryArgs.ball(r_max, r_min, exclude_ii=True)
    
    # start = time.time()
    # nlist = query.query(points, args)
    # elapsed = time.time() - start
    
    # print(f"Found {len(nlist)} neighbor pairs in range [{r_min}, {r_max}]")
    # print(f"Query time: {elapsed*1000:.2f} milliseconds")
    
    # Get distance statistics
    # distances = nlist.distances
    # if len(distances) > 0:
    #     print("Distance statistics:")
    #     print(f"  Min: {np.min(distances):.3f}")
    #     print(f"  Max: {np.max(distances):.3f}")
    #     print(f"  Mean: {np.mean(distances):.3f}\n")
    
    # Example 4: Working with neighbor lists
    print("Example 4: Working with Neighbor Lists")
    print("-" * 35)
    
    # # Filter by distance
    # nlist.filter_r(1.2, 0.8)
    # print(f"After filtering to range [0.8, 1.2]: {len(nlist)} pairs")
    
    # # Get neighbors for a specific point
    # point_idx = 0
    # neighbors = nlist.get_neighbors_for_point(point_idx)
    # print(f"Point {point_idx} has {len(neighbors)} neighbors")
    
    # # Access individual bonds
    # if len(nlist) > 0:
    #     bond = nlist[0]
    #     print(f"First bond: point {bond.query_point_idx} -> point {bond.point_idx}")
    #     print(f"  Distance: {bond.distance:.3f}")
    #     print(f"  Weight: {bond.weight}")
    
    print("\nPython example complete!")
    print("\nNote: This is a demonstration of the API.")
    print("To run actual calculations, compile the molcpp_locality module first:")
    print("  1. mkdir build && cd build")
    print("  2. cmake ../python -DCMAKE_PREFIX_PATH='path/to/xtensor;path/to/xtensor-python'")
    print("  3. make")
    print("  4. python test_aabb_python.py")


def demo_api_features():
    """
    Demonstrate additional API features.
    """
    print("\n" + "=" * 40)
    print("Additional API Features")
    print("=" * 40)
    
    print("\n1. Box operations:")
    print("   - box.wrap(position) - Wrap position into box")
    print("   - box.minimum_image(r_i, r_j) - Minimum image vector")
    print("   - box.distance(r_i, r_j) - Minimum image distance")
    print("   - box.set_periodic(x, y, z) - Set periodic boundaries")
    
    print("\n2. AABB operations:")
    print("   - molcpp_locality.overlap(aabb1, aabb2) - Check overlap")
    print("   - molcpp_locality.contains(aabb1, aabb2) - Check containment")
    print("   - molcpp_locality.merge(aabb1, aabb2) - Merge AABBs")
    
    print("\n3. Query modes:")
    print("   - QueryType.ball - Find all within radius")
    print("   - QueryType.nearest - Find k nearest neighbors")
    
    print("\n4. NeighborList arrays:")
    print("   - nlist.query_point_indices - Query point indices")
    print("   - nlist.point_indices - Neighbor point indices")
    print("   - nlist.distances - Distances between pairs")
    print("   - nlist.weights - Bond weights")
    print("   - nlist.distance_vectors - Distance vectors")


if __name__ == "__main__":
    demo_aabb_neighbor_finding()
    demo_api_features()