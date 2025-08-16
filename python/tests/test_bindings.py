#!/usr/bin/env python3
"""
Test Python bindings for molcpp ECS system
"""

import sys
import os

# Add the built module to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))

try:
    import molcpp
    print("✓ molcpp module imported successfully")
except ImportError as e:
    print(f"✗ Failed to import molcpp: {e}")
    sys.exit(1)


def test_ecs_basic():
    """Test basic ECS functionality without System"""
    print("\n=== Testing Basic ECS ===")
    
    # Test entity creation
    entity = molcpp.ecs.Entity()
    print(f"✓ Entity created with ID: {entity.get_id()}")
    
    # Test component addition
    pos = entity.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
    print(f"✓ Position component added: {pos}")
    
    # Test component query
    has_pos = entity.has_component(molcpp.ecs.Position)
    print(f"✓ Entity has Position: {has_pos}")


def test_components():
    """Test component creation and types"""
    print("\n=== Testing Components ===")
    
    # Test Position component
    pos = molcpp.ecs.Position(1.0, 2.0, 3.0)
    print(f"✓ Position created: x={pos.x}, y={pos.y}, z={pos.z}")
    print(f"✓ Position type: {pos.get_type_name()}")
    
    # Test Element component
    elem = molcpp.ecs.Element("C", 6)
    print(f"✓ Element created: {elem.symbol}, atomic_number={elem.atomic_number}")
    print(f"✓ Element type: {elem.get_type_name()}")
    
    # Test Mass component
    mass = molcpp.ecs.Mass(12.011)
    print(f"✓ Mass created: {mass.value}")
    print(f"✓ Mass type: {mass.get_type_name()}")
    
    # Test Radius component
    radius = molcpp.ecs.Radius(0.77)
    print(f"✓ Radius created: {radius.value}")
    print(f"✓ Radius type: {radius.get_type_name()}")


def test_entity_inheritance():
    """Test Entity base class functionality"""
    print("\n=== Testing Entity Base Class ===")
    
    # Create an Entity
    entity = molcpp.ecs.Entity()
    print(f"✓ Entity created with ID: {entity.get_id()}")
    
    # Test component management through Entity
    pos_comp = entity.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
    print(f"✓ Added Position component: {pos_comp}")
    
    # Test has_component
    has_pos = entity.has_component(molcpp.ecs.Position)
    print(f"✓ Entity has Position: {has_pos}")
    
    # Test get_component
    retrieved_pos = entity.get_component(molcpp.ecs.Position)
    print(f"✓ Retrieved Position: x={retrieved_pos.x}, y={retrieved_pos.y}, z={retrieved_pos.z}")
    
    # Test component removal
    removed = entity.remove_component(molcpp.ecs.Position)
    print(f"✓ Removed Position component: {removed}")
    
    has_pos_after = entity.has_component(molcpp.ecs.Position)
    print(f"✓ Entity has Position after removal: {has_pos_after}")


def test_atom_inheritance():
    """Test Atom inherits Entity functionality"""
    print("\n=== Testing Atom Inheritance ===")
    
    # Create Atom
    atom = molcpp.atom.Atom()
    print(f"✓ Atom created with ID: {atom.get_id()}")
    
    # Test that Atom inherits Entity methods
    # Add components through inherited methods
    atom.add_component(molcpp.ecs.Position, 0.0, 0.0, 0.0)
    atom.add_component(molcpp.ecs.Element, "C", 6)
    atom.add_component(molcpp.ecs.Mass, 12.011)
    atom.add_component(molcpp.ecs.Radius, 0.77)
    print("✓ Added components through inherited Entity methods")
    
    # Test convenience methods
    atom.set_position(1.5, 2.5, 3.5)
    pos_tuple = atom.get_position()
    print(f"✓ Set position via convenience method: {pos_tuple}")
    
    atom.set_element("O", 8)
    elem_tuple = atom.get_element()
    print(f"✓ Set element via convenience method: {elem_tuple}")
    
    atom.set_mass(15.999)
    mass_val = atom.get_mass()
    print(f"✓ Set mass via convenience method: {mass_val}")
    
    atom.set_radius(0.66)
    radius_val = atom.get_radius()
    print(f"✓ Set radius via convenience method: {radius_val}")
    
    print(f"✓ Atom representation: {atom}")


def test_bond_inheritance():
    """Test Bond inherits Entity functionality"""
    print("\n=== Testing Bond Inheritance ===")
    
    # Create two atoms
    atom1 = molcpp.atom.Atom()
    atom2 = molcpp.atom.Atom()
    print(f"✓ Created atoms with IDs: {atom1.get_id()}, {atom2.get_id()}")
    
    # Create bond between atoms
    bond = molcpp.bond.Bond(atom1, atom2)
    print(f"✓ Bond created with ID: {bond.get_id()}")
    
    # Test bond functionality
    atom_ids = bond.get_atoms()
    print(f"✓ Bond connects atoms: {atom_ids}")
    
    # Test that Bond inherits Entity methods
    bond.add_component(molcpp.ecs.Mass, 1.0)  # Example: bond "strength" as mass
    has_mass = bond.has_component(molcpp.ecs.Mass)
    print(f"✓ Bond has Mass component: {has_mass}")
    
    print(f"✓ Bond representation: {bond}")


def main():
    """Run all tests"""
    print("Testing molcpp Python bindings...")
    
    try:
        test_ecs_basic()
        test_components()
        test_entity_inheritance()
        test_atom_inheritance()
        test_bond_inheritance()
        
        print("\n🎉 All tests passed!")
        
    except Exception as e:
        print(f"\n💥 Test failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
