#!/usr/bin/env python3
"""
Test script for molcpp locality module Python bindings
"""

import numpy as np
from _bindings import locality

def test_basic_functionality():
    """Test basic AABBQuery functionality"""
    print("Testing basic AABBQuery functionality...")
    
    # Create a box
    origin = locality.Vec3(0.0, 0.0, 0.0)
    pbc = locality.Vec3(True, True, True)
    box = locality.create_box(10.0, origin, pbc)
    print(f"Created box with volume: {box.getVolume()}")
    
    # Create points on a grid
    points = locality.create_points_grid(3, 3, 3, 2.0)
    print(f"Created {points.shape[0]} points on a grid")
    
    # Create AABBQuery
    query = locality.AABBQuery(box, points)
    print(f"AABBQuery created with {query.getNPoints()} points")
    
    # Test query
    query_args = locality.QueryArgs()
    query_args.mode = locality.QueryType.ball
    query_args.r_max = 2.0
    query_args.r_min = 0.0
    query_args.exclude_ii = False
    
    # Query from center point
    center_point = locality.Vec3(2.0, 2.0, 2.0)
    iterator = query.querySingle(center_point, 13, query_args)
    
    if iterator:
        print("Query successful!")
        # Count neighbors
        neighbor_count = 0
        while not iterator.end():
            bond = iterator.next()
            if bond.getPointIdx() != -1:
                neighbor_count += 1
                print(f"  Found neighbor {bond.getPointIdx()} at distance {bond.getDistance():.3f}")
        
        print(f"Total neighbors found: {neighbor_count}")
    else:
        print("Query failed!")
    
    return True

def test_dynamic_update():
    """Test dynamic update functionality"""
    print("\nTesting dynamic update functionality...")
    
    # Create a box
    origin = locality.Vec3(0.0, 0.0, 0.0)
    pbc = locality.Vec3(True, True, True)
    box = locality.create_box(10.0, origin, pbc)
    
    # Create initial points
    initial_points = locality.create_points_grid(2, 2, 2, 3.0)
    print(f"Created initial {initial_points.shape[0]} points")
    
    # Create AABBQuery
    query = locality.AABBQuery(box, initial_points)
    print(f"Initial AABBQuery has {query.getNPoints()} points")
    
    # Create additional points
    new_points = np.zeros((2, 3), dtype=np.float32)
    new_points[0] = [4.5, 4.5, 4.5]
    new_points[1] = [4.5, 4.5, 1.5]
    
    # Update with new points
    query.update(new_points)
    print(f"After update, AABBQuery has {query.getNPoints()} points")
    
    # Test query from new point
    query_args = locality.QueryArgs()
    query_args.mode = locality.QueryType.ball
    query_args.r_max = 2.0
    query_args.r_min = 0.0
    query_args.exclude_ii = False
    
    new_point = locality.Vec3(4.5, 4.5, 4.5)
    iterator = query.querySingle(new_point, 8, query_args)
    
    if iterator:
        print("Query from new point successful!")
        neighbor_count = 0
        while not iterator.end():
            bond = iterator.next()
            if bond.getPointIdx() != -1:
                neighbor_count += 1
                print(f"  Found neighbor {bond.getPointIdx()} at distance {bond.getDistance():.3f}")
        
        print(f"Total neighbors found: {neighbor_count}")
    else:
        print("Query from new point failed!")
    
    return True

def test_neighbor_bond():
    """Test NeighborBond functionality"""
    print("\nTesting NeighborBond functionality...")
    
    # Create a NeighborBond
    vector = locality.Vec3(1.0, 0.0, 0.0)
    bond = locality.NeighborBond(0, 1, 1.0, 1.0, vector)
    
    print(f"Created bond: query={bond.getQueryPointIdx()}, point={bond.getPointIdx()}")
    print(f"Distance: {bond.getDistance():.3f}, Weight: {bond.getWeight()}")
    print(f"Vector: ({bond.getVector()[0]:.3f}, {bond.getVector()[1]:.3f}, {bond.getVector()[2]:.3f})")
    
    # Test setters
    bond.setQueryPointIdx(5)
    bond.setPointIdx(10)
    bond.setWeight(2.0)
    new_vector = locality.Vec3(0.0, 1.0, 0.0)
    bond.setVector(new_vector)
    
    print(f"After modification: query={bond.getQueryPointIdx()}, point={bond.getPointIdx()}")
    print(f"Distance: {bond.getDistance():.3f}, Weight: {bond.getWeight()}")
    print(f"Vector: ({bond.getVector()[0]:.3f}, {bond.getVector()[1]:.3f}, {bond.getVector()[2]:.3f})")
    
    return True

def test_aabb():
    """Test AABB functionality"""
    print("\nTesting AABB functionality...")
    
    # Create an AABB
    position = locality.Vec3(5.0, 5.0, 5.0)
    aabb = locality.AABB(position, 0)
    
    print(f"Created AABB at position: ({aabb.getPosition()[0]:.3f}, {aabb.getPosition()[1]:.3f}, {aabb.getPosition()[2]:.3f})")
    print(f"AABB tag: {aabb.getTag()}")
    print(f"AABB center: ({aabb.getCenter()[0]:.3f}, {aabb.getCenter()[1]:.3f}, {aabb.getCenter()[2]:.3f})")
    print(f"AABB volume: {aabb.getVolume():.3f}")
    print(f"AABB surface area: {aabb.getSurfaceArea():.3f}")
    
    return True

def main():
    """Run all tests"""
    print("Testing molcpp locality module Python bindings...")
    print("=" * 60)
    
    try:
        test_basic_functionality()
        test_dynamic_update()
        test_neighbor_bond()
        test_aabb()
        
        print("\n" + "=" * 60)
        print("All tests passed! 🎉")
        
    except Exception as e:
        print(f"\nTest failed with error: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    return True

if __name__ == "__main__":
    main()
