"""
pytest configuration and fixtures for molcpp tests
"""

import sys
import os
import pytest

# Add the built module to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'build'))

try:
    import molcpp
except ImportError as e:
    pytest.skip(f"molcpp module not available: {e}", allow_module_level=True)


@pytest.fixture
def ecs_system():
    """Fixture providing ECS System instance"""
    return molcpp.ecs.System.get_instance()


@pytest.fixture
def clean_entity():
    """Fixture providing a clean Entity instance"""
    return molcpp.ecs.Entity()


@pytest.fixture
def clean_atom():
    """Fixture providing a clean Atom instance"""
    return molcpp.atom.Atom()


@pytest.fixture
def atom_pair():
    """Fixture providing two clean Atom instances"""
    return molcpp.atom.Atom(), molcpp.atom.Atom()


@pytest.fixture
def clean_bond():
    """Fixture providing a clean Bond instance"""
    return molcpp.bond.Bond()


@pytest.fixture
def bond_with_atoms(atom_pair):
    """Fixture providing a Bond created with two atoms"""
    atom1, atom2 = atom_pair
    return molcpp.bond.Bond(atom1, atom2), atom1, atom2
