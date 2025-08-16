"""Test atom module functionality."""

import numpy as np
import pytest


def test_atom_import():
    """Test that atom module can be imported."""
    import molcpp
    assert hasattr(molcpp, 'atom')


def test_atom_creation():
    """Test basic Atom creation."""
    import molcpp
    
    # Create an atom
    atom = molcpp.atom.Atom()
    assert atom is not None
    
    # Test that it has an ID
    atom_id = atom.get_id()
    assert isinstance(atom_id, int)
    assert atom_id >= 0
    
    # Test string representation
    repr_str = repr(atom)
    assert isinstance(repr_str, str)
    assert 'Atom' in repr_str
    assert str(atom_id) in repr_str


def test_atom_position_component():
    """Test adding and managing Position component."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Initially should not have position
    assert not atom.has_position()
    assert atom.get_position() is None
    
    # Add position component
    pos = atom.add_position(1.0, 2.0, 3.0)
    assert atom.has_position()
    assert pos.x == 1.0
    assert pos.y == 2.0
    assert pos.z == 3.0
    
    # Get position component
    retrieved_pos = atom.get_position()
    assert retrieved_pos is not None
    assert retrieved_pos.x == 1.0
    assert retrieved_pos.y == 2.0
    assert retrieved_pos.z == 3.0


def test_atom_position_from_numpy():
    """Test adding Position component from numpy array."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Add position from numpy array
    arr = np.array([4.0, 5.0, 6.0])
    pos = atom.add_position_from_array(arr)
    assert pos.x == 4.0
    assert pos.y == 5.0
    assert pos.z == 6.0
    
    # Verify it's the same as getting the component
    retrieved_pos = atom.get_position()
    assert retrieved_pos.x == 4.0
    assert retrieved_pos.y == 5.0
    assert retrieved_pos.z == 6.0


def test_atom_element_component():
    """Test adding and managing Element component."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Initially should not have element
    assert not atom.has_element()
    assert atom.get_element() is None
    
    # Add element component
    elem = atom.add_element("C", 6)
    assert atom.has_element()
    assert elem.symbol == "C"
    assert elem.atomic_number == 6
    
    # Get element component
    retrieved_elem = atom.get_element()
    assert retrieved_elem is not None
    assert retrieved_elem.symbol == "C"
    assert retrieved_elem.atomic_number == 6


def test_atom_velocity_component():
    """Test adding and managing Velocity component."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Add velocity component
    vel = atom.add_velocity(0.1, 0.2, 0.3)
    assert atom.has_velocity()
    assert vel.vx == 0.1
    assert vel.vy == 0.2
    assert vel.vz == 0.3
    
    # Add velocity from numpy array
    atom2 = molcpp.atom.Atom()
    arr = np.array([0.4, 0.5, 0.6])
    vel2 = atom2.add_velocity_from_array(arr)
    assert vel2.vx == 0.4
    assert vel2.vy == 0.5
    assert vel2.vz == 0.6


def test_atom_scalar_components():
    """Test scalar components (radius, mass, charge)."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Add radius
    radius = atom.add_radius(1.5)
    assert atom.has_radius()
    assert radius.value == 1.5
    
    # Add mass
    mass = atom.add_mass(12.0)
    assert atom.has_mass()
    assert mass.value == 12.0
    
    # Add charge
    charge = atom.add_charge(-0.5)
    assert atom.has_charge()
    assert charge.value == -0.5


def test_atom_component_removal():
    """Test removing components."""
    import molcpp
    
    atom = molcpp.atom.Atom()
    
    # Add components
    atom.add_position(1.0, 2.0, 3.0)
    atom.add_element("H", 1)
    atom.add_radius(1.0)
    
    # Verify they exist
    assert atom.has_position()
    assert atom.has_element()
    assert atom.has_radius()
    
    # Remove components
    assert atom.remove_position()
    assert atom.remove_element()
    assert atom.remove_radius()
    
    # Verify they're gone
    assert not atom.has_position()
    assert not atom.has_element()
    assert not atom.has_radius()
    
    # Try removing again (should return False)
    assert not atom.remove_position()


def test_multiple_atoms():
    """Test creating multiple atoms with different components."""
    import molcpp
    
    # Create two atoms
    atom1 = molcpp.atom.Atom()
    atom2 = molcpp.atom.Atom()
    
    # They should have different IDs
    assert atom1.get_id() != atom2.get_id()
    
    # Add different components to each
    atom1.add_position(1.0, 2.0, 3.0)
    atom1.add_element("C", 6)
    
    atom2.add_position(4.0, 5.0, 6.0)
    atom2.add_element("N", 7)
    
    # Verify components are separate
    pos1 = atom1.get_position()
    pos2 = atom2.get_position()
    assert pos1.x != pos2.x
    
    elem1 = atom1.get_element()
    elem2 = atom2.get_element()
    assert elem1.symbol != elem2.symbol