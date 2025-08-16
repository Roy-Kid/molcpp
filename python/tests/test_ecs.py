"""Test ECS module functionality."""

import numpy as np
import pytest


def test_ecs_import():
    """Test that ecs module can be imported."""
    import molcpp
    assert hasattr(molcpp, 'ecs')
    assert hasattr(molcpp.ecs, 'components')


def test_entity_creation():
    """Test basic Entity creation."""
    import molcpp
    
    # Create an entity
    entity = molcpp.ecs.Entity()
    assert entity is not None
    
    # Test that it has an ID
    entity_id = entity.get_id()
    assert isinstance(entity_id, int)
    assert entity_id >= 0
    
    # Test string representation
    repr_str = repr(entity)
    assert isinstance(repr_str, str)
    assert 'Entity' in repr_str
    assert str(entity_id) in repr_str


def test_position_component():
    """Test Position component."""
    import molcpp
    
    # Create with default values
    pos1 = molcpp.ecs.components.Position()
    assert pos1.x == 0.0
    assert pos1.y == 0.0
    assert pos1.z == 0.0
    
    # Create with specific values
    pos2 = molcpp.ecs.components.Position(1.0, 2.0, 3.0)
    assert pos2.x == 1.0
    assert pos2.y == 2.0
    assert pos2.z == 3.0
    
    # Create from numpy array
    arr = np.array([4.0, 5.0, 6.0])
    pos3 = molcpp.ecs.components.Position(arr)
    assert pos3.x == 4.0
    assert pos3.y == 5.0
    assert pos3.z == 6.0
    
    # Test conversion to array
    result_arr = pos3.to_array()
    assert isinstance(result_arr, np.ndarray)
    assert result_arr.shape == (3,)
    np.testing.assert_allclose(result_arr, [4.0, 5.0, 6.0])
    
    # Test from_array method
    new_arr = np.array([7.0, 8.0, 9.0])
    pos3.from_array(new_arr)
    assert pos3.x == 7.0
    assert pos3.y == 8.0
    assert pos3.z == 9.0


def test_element_component():
    """Test Element component."""
    import molcpp
    
    # Create empty element
    elem1 = molcpp.ecs.components.Element()
    assert elem1.symbol == ""
    assert elem1.atomic_number == 0
    
    # Create with values
    elem2 = molcpp.ecs.components.Element("C", 6)
    assert elem2.symbol == "C"
    assert elem2.atomic_number == 6
    
    # Test modification
    elem2.symbol = "N"
    elem2.atomic_number = 7
    assert elem2.symbol == "N"
    assert elem2.atomic_number == 7


def test_velocity_component():
    """Test Velocity component with numpy arrays."""
    import molcpp
    
    # Create with default values
    vel1 = molcpp.ecs.components.Velocity()
    assert vel1.vx == 0.0
    assert vel1.vy == 0.0
    assert vel1.vz == 0.0
    
    # Create with specific values
    vel2 = molcpp.ecs.components.Velocity(1.5, 2.5, 3.5)
    assert vel2.vx == 1.5
    assert vel2.vy == 2.5
    assert vel2.vz == 3.5
    
    # Create from numpy array
    arr = np.array([0.1, 0.2, 0.3])
    vel3 = molcpp.ecs.components.Velocity(arr)
    assert vel3.vx == 0.1
    assert vel3.vy == 0.2
    assert vel3.vz == 0.3
    
    # Test array conversions
    result_arr = vel3.to_array()
    assert isinstance(result_arr, np.ndarray)
    np.testing.assert_allclose(result_arr, [0.1, 0.2, 0.3])


def test_scalar_components():
    """Test scalar components (Radius, Mass, Charge)."""
    import molcpp
    
    # Radius
    radius = molcpp.ecs.components.Radius(1.5)
    assert radius.value == 1.5
    
    # Mass
    mass = molcpp.ecs.components.Mass(12.0)
    assert mass.value == 12.0
    
    # Charge
    charge = molcpp.ecs.components.Charge(-1.0)
    assert charge.value == -1.0
