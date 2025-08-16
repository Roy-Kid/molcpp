# conftest.py
# Test configuration for molcpp Python tests

import sys
import os

# Add the source directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src'))

import pytest
import numpy as np


@pytest.fixture
def sample_coords():
    """Sample 3D coordinates for testing."""
    return np.array([
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
        [0.0, 0.0, 1.0]
    ])


@pytest.fixture
def sample_vec3():
    """Sample Vec3 for testing."""
    return np.array([1.0, 2.0, 3.0])


@pytest.fixture
def sample_mat3():
    """Sample Mat3 for testing."""
    return np.array([
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0],
        [0.0, 0.0, 1.0]
    ])
