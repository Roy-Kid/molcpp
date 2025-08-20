"""
Pytest configuration for molcpp tests.
"""

import sys
import os
import pytest

# Add the molcpp Python module to the path
sys.path.insert(0, '/workspaces/molcrafts-1/molcpp/python/src/molcpp')

# Import the bindings module
import _bindings

# Make bindings available to all tests
@pytest.fixture(scope="session")
def bindings():
    """Provide the molcpp bindings module to all tests."""
    return _bindings


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
