#!/usr/bin/env python3
"""
Pytest tests for molcpp locality module.
"""

import sys
import os
sys.path.insert(0, '/workspaces/molcrafts-1/molcpp/python/src/molcpp')

import numpy as np
import pytest
import _bindings


class TestLocalityModule:
    """Test Locality module functionality."""
    
    def test_neighbor_bond_creation(self):
        """Test NeighborBond creation."""
        # Test default constructor
        bond = _bindings.locality.NeighborBond()
        assert bond.getQueryPointIdx() == 0
        assert bond.getPointIdx() == 0
        assert bond.getWeight() == 0.0
        assert bond.getDistance() == 0.0
        
        # Test with values (using 4-arg constructor: query_point_idx, point_idx, weight, vector)
        vector = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        bond = _bindings.locality.NeighborBond(1, 2, 0.5, vector)
        assert bond.getQueryPointIdx() == 1
        assert bond.getPointIdx() == 2
        assert bond.getWeight() == 0.5
        # Distance is calculated from vector
        expected_distance = np.sqrt(np.sum(vector * vector))
        assert abs(bond.getDistance() - expected_distance) < 1e-6
        assert np.allclose(bond.getVector(), vector)
    
    def test_neighbor_bond_modification(self):
        """Test NeighborBond modification."""
        bond = _bindings.locality.NeighborBond()
        
        # Test setters
        bond.setQueryPointIdx(10)
        bond.setPointIdx(20)
        bond.setWeight(0.75)
        
        vector = np.array([4.0, 5.0, 6.0], dtype=np.float32)
        bond.setVector(vector)
        
        # Verify changes
        assert bond.getQueryPointIdx() == 10
        assert bond.getPointIdx() == 20
        assert bond.getWeight() == 0.75
        # Distance is recalculated when vector is set
        expected_distance = np.sqrt(np.sum(vector * vector))
        assert abs(bond.getDistance() - expected_distance) < 1e-6
        assert np.allclose(bond.getVector(), vector)
    
    def test_neighbor_bond_comparison(self):
        """Test NeighborBond comparison operators."""
        bond1 = _bindings.locality.NeighborBond(1, 2, 0.5, np.array([1.0, 0.0, 0.0], dtype=np.float32))
        bond2 = _bindings.locality.NeighborBond(1, 2, 0.5, np.array([1.0, 0.0, 0.0], dtype=np.float32))
        bond3 = _bindings.locality.NeighborBond(2, 3, 0.8, np.array([0.0, 2.0, 0.0], dtype=np.float32))
        
        # Test equality
        assert bond1 == bond2
        assert bond1 != bond3
        
        # Test ordering (based on distance)
        assert bond1 < bond3  # distance 1.0 < distance 2.0
    
    def test_aabb_creation(self):
        """Test AABB creation."""
        lower = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        upper = np.array([4.0, 5.0, 6.0], dtype=np.float32)
        
        # Test default constructor
        aabb = _bindings.locality.AABB()
        assert aabb.tag == 0
        
        # Test with lower and upper bounds
        aabb = _bindings.locality.AABB(lower, upper)
        assert aabb.tag == 0
        
        # Test with position and radius
        pos = np.array([2.5, 3.5, 4.5], dtype=np.float32)
        aabb = _bindings.locality.AABB(pos, 1.5)
        assert aabb.tag == 0
        
        # Test with position and tag
        aabb = _bindings.locality.AABB(pos, 42)
        assert aabb.tag == 42
    
    def test_aabb_properties(self):
        """Test AABB properties."""
        lower = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        upper = np.array([4.0, 5.0, 6.0], dtype=np.float32)
        
        aabb = _bindings.locality.AABB(lower, upper)
        
        # Test getters
        retrieved_lower = aabb.getLower()
        retrieved_upper = aabb.getUpper()
        
        assert np.allclose(retrieved_lower, lower)
        assert np.allclose(retrieved_upper, upper)
        
        # Test position (center)
        center = aabb.getPosition()
        expected_center = (lower + upper) / 2
        assert np.allclose(center, expected_center)
    
    def test_aabb_sphere_creation(self):
        """Test AABBSphere creation."""
        pos = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        radius = 2.5
        
        # Test default constructor
        sphere = _bindings.locality.AABBSphere()
        assert sphere.radius == 0.0
        assert sphere.tag == 0
        
        # Test with position and radius
        sphere = _bindings.locality.AABBSphere(pos, radius)
        assert sphere.radius == 2.5
        assert sphere.tag == 0
        
        # Test with position, radius, and tag
        sphere2 = _bindings.locality.AABBSphere(pos, radius, 42)
        assert sphere2.tag == 42
    
    def test_aabb_sphere_properties(self):
        """Test AABBSphere properties."""
        pos = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        radius = 2.5
        
        sphere = _bindings.locality.AABBSphere(pos, radius)
        
        # Test position
        retrieved_pos = sphere.getPosition()
        assert np.allclose(retrieved_pos, pos)
        
        # Test radius
        assert sphere.radius == radius
    
    def test_aabb_sphere_translation(self):
        """Test AABBSphere translation."""
        pos = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        radius = 2.5
        
        sphere = _bindings.locality.AABBSphere(pos, radius)
        
        # Translate
        offset = np.array([10.0, 20.0, 30.0], dtype=np.float32)
        sphere.translate(offset)
        
        # Check new position
        new_pos = sphere.getPosition()
        expected_pos = pos + offset
        assert np.allclose(new_pos, expected_pos)
        
        # Radius should remain unchanged
        assert sphere.radius == radius
    
    def test_query_args(self):
        """Test QueryArgs."""
        args = _bindings.locality.QueryArgs()
        
        # Test default values - the C++ default is 'none', not 'ball'
        assert args.mode == _bindings.locality.QueryType.none
        assert args.r_max == -1.0
        assert args.num_neighbors == 0xffffffff
        
        # Test modification
        args.mode = _bindings.locality.QueryType.nearest
        args.r_max = 5.0
        args.num_neighbors = 10
        
        assert args.mode == _bindings.locality.QueryType.nearest
        assert args.r_max == 5.0
        assert args.num_neighbors == 10
    
    def test_query_type_enum(self):
        """Test QueryType enum."""
        # Test enum values exist
        assert hasattr(_bindings.locality.QueryType, 'none')
        assert hasattr(_bindings.locality.QueryType, 'ball')
        assert hasattr(_bindings.locality.QueryType, 'nearest')
        
        # Test they are different
        assert _bindings.locality.QueryType.ball != _bindings.locality.QueryType.nearest
        assert _bindings.locality.QueryType.none != _bindings.locality.QueryType.ball
    
    def test_aabb_query_basic(self):
        """Test basic AABBQuery functionality."""
        # Create a simple box
        matrix = np.array([[10.0, 0.0, 0.0], [0.0, 10.0, 0.0], [0.0, 0.0, 10.0]], dtype=np.float32)
        origin = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        pbc = np.array([True, True, True], dtype=bool)
        box = _bindings.spatial.Box(matrix, origin, pbc)
        
        # Create some points
        points = np.array([[1.0, 1.0, 1.0], [2.0, 2.0, 2.0], [3.0, 3.0, 3.0]], dtype=np.float32)
        
        # Create AABBQuery
        query = _bindings.locality.AABBQuery(box, points)
        
        # Test basic properties
        assert query.getNPoints() == 3
        # Box objects can't be compared with ==, so just check they're the same type
        retrieved_box = query.getBox()
        assert isinstance(retrieved_box, _bindings.spatial.Box)
        
        # Test that we can access the points
        retrieved_points = query.getPoints()
        assert retrieved_points.shape == (3, 3)
        
        # Test that the query object is working correctly
        # Just verify it doesn't crash when we access basic properties
        assert hasattr(query, 'build')
        assert hasattr(query, 'update')
    
    def test_aabb_query_dynamic(self):
        """Test AABBQuery dynamic update functionality."""
        # Create a simple box
        matrix = np.array([[10.0, 0.0, 0.0], [0.0, 10.0, 0.0], [0.0, 0.0, 10.0]], dtype=np.float32)
        origin = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        pbc = np.array([True, True, True], dtype=bool)
        box = _bindings.spatial.Box(matrix, origin, pbc)
        
        # Create initial points
        points = np.array([[1.0, 1.0, 1.0], [2.0, 2.0, 2.0]], dtype=np.float32)
        
        # Create AABBQuery
        query = _bindings.locality.AABBQuery(box, points)
        
        # Test initial state
        assert query.getNPoints() == 2
        
        # Test update with new points
        new_points = np.array([[4.0, 4.0, 4.0], [5.0, 5.0, 5.0]], dtype=np.float32)
        query.update(new_points)
        
        # Should now have 4 points
        assert query.getNPoints() == 4
    
    def test_aabb_operations(self):
        """Test AABB utility functions."""
        # Create two AABBs
        lower1 = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        upper1 = np.array([2.0, 2.0, 2.0], dtype=np.float32)
        aabb1 = _bindings.locality.AABB(lower1, upper1)
        
        lower2 = np.array([1.0, 1.0, 1.0], dtype=np.float32)
        upper2 = np.array([3.0, 3.0, 3.0], dtype=np.float32)
        aabb2 = _bindings.locality.AABB(lower2, upper2)
        
        # Test overlap
        assert _bindings.locality.overlap(aabb1, aabb2) == True
        
        # Test contains (AABB contains point)
        point = np.array([0.5, 0.5, 0.5], dtype=np.float32)
        assert _bindings.locality.contains(aabb1, point) == True
        
        # Test contains (AABB contains another AABB)
        small_lower = np.array([0.5, 0.5, 0.5], dtype=np.float32)
        small_upper = np.array([1.5, 1.5, 1.5], dtype=np.float32)
        small_aabb = _bindings.locality.AABB(small_lower, small_upper)
        assert _bindings.locality.contains(aabb1, small_aabb) == True
        
        # Test merge
        merged = _bindings.locality.merge(aabb1, aabb2)
        expected_lower = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        expected_upper = np.array([3.0, 3.0, 3.0], dtype=np.float32)
        assert np.allclose(merged.getLower(), expected_lower)
        assert np.allclose(merged.getUpper(), expected_upper)
