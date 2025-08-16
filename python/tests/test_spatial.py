"""Test spatial module functionality"""

import numpy as np
import molcpp
from molcpp.spatial import Box, BoxStyle


class TestBoxBasics:
    """Test basic Box functionality"""
    
    def test_box_creation(self):
        """Test creating boxes with different methods"""
        # Test free box
        box = Box()
        assert box.get_style() == BoxStyle.FREE
        
        # Test from lengths list
        box = Box([10.0, 10.0, 10.0])
        assert box.get_style() == BoxStyle.ORTHOGONAL
        lengths = box.get_lengths()
        np.testing.assert_array_almost_equal(lengths, [10.0, 10.0, 10.0])
        
    def test_box_properties(self):
        """Test basic box properties"""
        box = Box([5.0, 6.0, 7.0])
        
        # Test lengths
        lengths = box.get_lengths()
        np.testing.assert_array_almost_equal(lengths, [5.0, 6.0, 7.0])
        
        # Test volume
        volume = box.get_volume()
        assert abs(volume - 5.0 * 6.0 * 7.0) < 1e-10
        
    def test_box_matrix(self):
        """Test box matrix operations"""
        box = Box([10.0, 10.0, 10.0])
        
        # Test matrix
        matrix = box.get_matrix()
        expected = np.array([[10.0, 0.0, 0.0],
                           [0.0, 10.0, 0.0],
                           [0.0, 0.0, 10.0]])
        np.testing.assert_array_almost_equal(matrix, expected)
        
        # Test inverse matrix
        inv_matrix = box.get_inv()
        expected_inv = np.array([[0.1, 0.0, 0.0],
                               [0.0, 0.1, 0.0],
                               [0.0, 0.0, 0.1]])
        np.testing.assert_array_almost_equal(inv_matrix, expected_inv)


class TestBoxWrapping:
    """Test coordinate wrapping functionality"""
    
    def test_wrap_orthogonal(self):
        """Test wrapping in orthogonal box"""
        box = Box([10.0, 10.0, 10.0])
        
        # Test single particle
        coords = np.array([[15.0, -5.0, 2.0]])
        wrapped = box.wrap(coords)
        expected = np.array([[5.0, 5.0, 2.0]])
        np.testing.assert_array_almost_equal(wrapped, expected)
        
        # Test multiple particles
        coords = np.array([[15.0, -5.0, 2.0],
                          [3.0, 12.0, -1.0]])
        wrapped = box.wrap(coords)
        expected = np.array([[5.0, 5.0, 2.0],
                           [3.0, 2.0, 9.0]])
        np.testing.assert_array_almost_equal(wrapped, expected)
        
    def test_minimum_image(self):
        """Test minimum image calculations"""
        box = Box([10.0, 10.0, 10.0])
        
        # Test minimum image distance
        r1 = np.array([[1.0, 1.0, 1.0]])
        r2 = np.array([[9.0, 9.0, 9.0]])
        dr = box.minimum_image(r1, r2)
        
        # Should wrap around to shorter distance
        expected = np.array([[-2.0, -2.0, -2.0]])
        np.testing.assert_array_almost_equal(dr, expected, decimal=5)


class TestBoxFactories:
    """Test static factory methods"""
    
    def test_from_lengths_angles(self):
        """Test creating box from lengths and angles"""
        lengths = np.array([5.0, 6.0, 7.0])
        angles = np.array([90.0, 90.0, 90.0])  # orthogonal
        
        box = Box.from_lengths_angles(lengths, angles)
        
        # Should be orthogonal
        assert box.get_style() == BoxStyle.ORTHOGONAL
        
        box_lengths = box.get_lengths()
        np.testing.assert_array_almost_equal(box_lengths, lengths)


class TestRegionInterface:
    """Test Box as Region"""
    
    def test_isin_orthogonal(self):
        """Test point-in-box testing for orthogonal box"""
        box = Box([10.0, 10.0, 10.0])
        
        # Test points inside and outside
        coords = np.array([[5.0, 5.0, 5.0],   # inside
                          [15.0, 5.0, 5.0],   # outside x
                          [-1.0, 5.0, 5.0],   # outside x (negative)
                          [0.0, 0.0, 0.0],    # on boundary
                          [10.0, 10.0, 10.0]])  # on boundary
        
        inside = box.isin(coords)
        
        # Check results - depending on implementation, boundary points may or may not be included
        assert inside[0] == True   # clearly inside
        assert inside[1] == False  # clearly outside
        assert inside[2] == False  # clearly outside
        
    def test_boundary_method(self):
        """Test boundary method from Region interface"""
        box = Box([10.0, 12.0, 8.0])
        bounds = box.boundary()
        
        # Should return [xmin, xmax, ymin, ymax, zmin, zmax]
        expected = [0.0, 10.0, 0.0, 12.0, 0.0, 8.0]
        assert len(bounds) == 6
        for i in range(6):
            assert abs(bounds[i] - expected[i]) < 1e-10


class TestBoundaryInterface:
    """Test Box as Boundary"""
    
    def test_boundary_methods(self):
        """Test boundary-specific methods"""
        box = Box([10.0, 10.0, 10.0])
        
        # Test get_bounds
        bounds = box.get_bounds()
        assert len(bounds) == 6
        
        # Test periodicity
        periodic = box.is_periodic()
        assert len(periodic) == 3
        # For orthogonal box, should be periodic in all directions
        assert all(periodic)


def test_box_styles():
    """Test BoxStyle enum"""
    # Test enum values exist
    assert hasattr(BoxStyle, 'FREE')
    assert hasattr(BoxStyle, 'ORTHOGONAL') 
    assert hasattr(BoxStyle, 'TRICLINIC')
    
    # Test they are different
    assert BoxStyle.FREE != BoxStyle.ORTHOGONAL
    assert BoxStyle.ORTHOGONAL != BoxStyle.TRICLINIC
