#!/usr/bin/env python3
"""
Simple pytest tests for molcpp ECS module.
"""

import sys
import os
sys.path.insert(0, '/workspaces/molcrafts-1/molcpp/python/src/molcpp')

import numpy as np
import pytest
import _bindings


def test_ecs_module_import():
    """Test that ECS module can be imported."""
    assert hasattr(_bindings, 'ecs')
    assert hasattr(_bindings.ecs, 'Entity')
    assert hasattr(_bindings.ecs, 'Position')
    assert hasattr(_bindings.ecs, 'Element')


def test_entity_creation():
    """Test Entity creation."""
    entity = _bindings.ecs.Entity()
    assert entity.get_id() > 0


def test_position_component():
    """Test Position component."""
    entity = _bindings.ecs.Entity()
    pos = entity.add_position(1.0, 2.0, 3.0)
    assert pos.x == 1.0
    assert pos.y == 2.0
    assert pos.z == 3.0


def test_element_component():
    """Test Element component."""
    entity = _bindings.ecs.Entity()
    elem = entity.add_element('C', 6)
    assert elem.symbol == 'C'
    assert elem.atomic_number == 6


def test_mass_component():
    """Test Mass component."""
    entity = _bindings.ecs.Entity()
    mass = entity.add_mass(12.0)
    assert mass.value == 12.0


def test_radius_component():
    """Test Radius component."""
    entity = _bindings.ecs.Entity()
    radius = entity.add_radius(1.7)
    assert radius.value == 1.7


def test_velocity_component():
    """Test Velocity component."""
    entity = _bindings.ecs.Entity()
    vel = entity.add_velocity(0.1, 0.2, 0.3)
    assert vel.vx == 0.1
    assert vel.vy == 0.2
    assert vel.vz == 0.3


def test_charge_component():
    """Test Charge component."""
    entity = _bindings.ecs.Entity()
    charge = entity.add_charge(-0.5)
    assert charge.value == -0.5


def test_bond_info_component():
    """Test BondInfo component."""
    entity = _bindings.ecs.Entity()
    bond = entity.add_bond_info(1, 2)
    assert bond.atom1_id == 1
    assert bond.atom2_id == 2


def test_component_equality():
    """Test component equality operators."""
    pos1 = _bindings.ecs.Position(1.0, 2.0, 3.0)
    pos2 = _bindings.ecs.Position(1.0, 2.0, 3.0)
    pos3 = _bindings.ecs.Position(1.0, 2.0, 4.0)
    
    assert pos1 == pos2
    assert pos1 != pos3
    
    elem1 = _bindings.ecs.Element('C', 6)
    elem2 = _bindings.ecs.Element('C', 6)
    elem3 = _bindings.ecs.Element('H', 1)
    
    assert elem1 == elem2
    assert elem1 != elem3


def test_spatial_module_import():
    """Test that Spatial module can be imported."""
    assert hasattr(_bindings, 'spatial')
    assert hasattr(_bindings.spatial, 'Box')


def test_locality_module_import():
    """Test that Locality module can be imported."""
    assert hasattr(_bindings, 'locality')
    assert hasattr(_bindings.locality, 'NeighborBond')
    assert hasattr(_bindings.locality, 'AABB')
    assert hasattr(_bindings.locality, 'AABBQuery')
