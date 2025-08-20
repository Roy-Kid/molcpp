#!/usr/bin/env python3
"""
Pytest tests for molcpp Spatial module.
"""

import sys
import os
sys.path.insert(0, '/workspaces/molcrafts-1/molcpp/python/src/molcpp')

import numpy as np
import pytest
import _bindings


class TestSpatialModule:
    """Test Spatial module functionality."""
    
    def test_box_cube_creation(self):
        """Test Box.cube factory method."""
        box = _bindings.spatial.Box.cube(
            10.0,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        
        assert abs(box.getVolume() - 1000.0) < 1e-6
        assert box.getVolume() == 1000.0
    
    def test_box_orthorhombic_creation(self):
        """Test Box.orthorhombic factory method."""
        lengths = np.array([5.0, 6.0, 7.0], dtype=np.float32)
        box = _bindings.spatial.Box.orthorhombic(
            lengths,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        
        assert abs(box.getVolume() - 210.0) < 1e-6
        assert box.getVolume() == 210.0
    
    def test_box_properties(self):
        """Test Box properties."""
        box = _bindings.spatial.Box.cube(
            10.0,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        
        # Test origin
        origin = box.origin()
        assert origin.shape == (3,)
        assert np.allclose(origin, np.array([0.0, 0.0, 0.0]))
        
        # Test PBC
        pbc = box.pbc()
        assert pbc.shape == (3,)
        assert np.all(pbc == True)
        
        # Test matrix
        matrix = box.matrix()
        assert matrix.shape == (3, 3)
    
    def test_box_volume_calculations(self):
        """Test Box volume calculations."""
        # Test different cube sizes
        sizes = [1.0, 2.0, 5.0, 10.0]
        for size in sizes:
            box = _bindings.spatial.Box.cube(
                size,
                np.array([0.0, 0.0, 0.0], dtype=np.float32),
                np.array([True, True, True], dtype=bool)
            )
            expected_volume = size ** 3
            assert abs(box.getVolume() - expected_volume) < 1e-6
    
    def test_box_origin_variations(self):
        """Test Box with different origins."""
        origins = [
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([-5.0, -5.0, -5.0], dtype=np.float32),
            np.array([10.0, 20.0, 30.0], dtype=np.float32)
        ]
        
        for origin in origins:
            box = _bindings.spatial.Box.cube(
                10.0,
                origin,
                np.array([True, True, True], dtype=bool)
            )
            
            retrieved_origin = box.origin()
            assert np.allclose(retrieved_origin, origin)
            assert box.getVolume() == 1000.0  # Volume should be independent of origin
    
    def test_box_pbc_variations(self):
        """Test Box with different periodic boundary conditions."""
        pbc_configs = [
            np.array([True, True, True], dtype=bool),   # All periodic
            np.array([False, False, False], dtype=bool), # All non-periodic
            np.array([True, False, True], dtype=bool),   # Mixed
        ]
        
        for pbc in pbc_configs:
            box = _bindings.spatial.Box.cube(
                10.0,
                np.array([0.0, 0.0, 0.0], dtype=np.float32),
                pbc
            )
            
            retrieved_pbc = box.pbc()
            assert np.all(retrieved_pbc == pbc)
            assert box.getVolume() == 1000.0  # Volume should be independent of PBC
    
    def test_box_matrix_properties(self):
        """Test Box matrix properties."""
        box = _bindings.spatial.Box.cube(
            10.0,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        
        matrix = box.matrix()
        assert matrix.shape == (3, 3)
        
        # For a cube, the matrix should be diagonal with the size
        expected_matrix = np.array([
            [10.0, 0.0, 0.0],
            [0.0, 10.0, 0.0],
            [0.0, 0.0, 10.0]
        ], dtype=np.float32)
        
        assert np.allclose(matrix, expected_matrix)
    
    def test_box_edge_cases(self):
        """Test Box edge cases."""
        # Test very small box
        small_box = _bindings.spatial.Box.cube(
            0.001,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        assert abs(small_box.getVolume() - 1e-9) < 1e-12
        
        # Test very large box
        large_box = _bindings.spatial.Box.cube(
            1000.0,
            np.array([0.0, 0.0, 0.0], dtype=np.float32),
            np.array([True, True, True], dtype=bool)
        )
        assert abs(large_box.getVolume() - 1e9) < 1e6
    
    def test_box_orthorhombic_variations(self):
        """Test Box.orthorhombic with different dimensions."""
        dimensions = [
            (1.0, 1.0, 1.0),      # Cube
            (2.0, 3.0, 4.0),      # Different lengths
            (0.5, 1.0, 2.0),      # Small to large
            (10.0, 20.0, 30.0),   # Large dimensions
        ]
        
        for lx, ly, lz in dimensions:
            lengths = np.array([lx, ly, lz], dtype=np.float32)
            box = _bindings.spatial.Box.orthorhombic(
                lengths,
                np.array([0.0, 0.0, 0.0], dtype=np.float32),
                np.array([True, True, True], dtype=bool)
            )
            
            expected_volume = lx * ly * lz
            assert abs(box.getVolume() - expected_volume) < 1e-6
            
            # Check matrix is diagonal with correct lengths
            matrix = box.matrix()
            assert abs(matrix[0, 0] - lx) < 1e-6
            assert abs(matrix[1, 1] - ly) < 1e-6
            assert abs(matrix[2, 2] - lz) < 1e-6
