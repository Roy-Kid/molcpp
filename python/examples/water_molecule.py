#!/usr/bin/env python3
"""
Example usage of molcpp Python bindings
Demonstrates the ECS architecture with inheritance
"""

import sys
import os

# Add the built module to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))

import molcpp


def create_water_molecule():
    """Create a simple water molecule using the ECS system"""
    print("Creating a water molecule...")
    
    # Create oxygen atom
    oxygen = molcpp.atom.Atom()
    oxygen.add_component(molcpp.ecs.Position, 0.0, 0.0, 0.0)
    oxygen.add_component(molcpp.ecs.Element, "O", 8)
    oxygen.add_component(molcpp.ecs.Mass, 15.999)
    oxygen.add_component(molcpp.ecs.Radius, 0.66)
    print(f"Oxygen: {oxygen}")
    
    # Create first hydrogen atom
    hydrogen1 = molcpp.atom.Atom()
    hydrogen1.set_position(0.96, 0.0, 0.0)  # Using convenience method
    hydrogen1.set_element("H", 1)
    hydrogen1.set_mass(1.008)
    hydrogen1.set_radius(0.31)
    print(f"Hydrogen 1: {hydrogen1}")
    
    # Create second hydrogen atom
    hydrogen2 = molcpp.atom.Atom()
    hydrogen2.set_position(-0.24, 0.93, 0.0)
    hydrogen2.set_element("H", 1)
    hydrogen2.set_mass(1.008)
    hydrogen2.set_radius(0.31)
    print(f"Hydrogen 2: {hydrogen2}")
    
    # Create bonds
    bond1 = molcpp.bond.Bond(oxygen, hydrogen1)
    bond2 = molcpp.bond.Bond(oxygen, hydrogen2)
    print(f"O-H Bond 1: {bond1}")
    print(f"O-H Bond 2: {bond2}")
    
    return oxygen, hydrogen1, hydrogen2, bond1, bond2


def analyze_system():
    """Analyze the current ECS system state"""
    print("\nAnalyzing ECS system...")
    
    system = molcpp.ecs.System.get_instance()
    print(f"Total entities: {system.get_entity_count()}")
    
    # Count components
    pos_count = system.get_component_number(molcpp.ecs.Position)
    elem_count = system.get_component_number(molcpp.ecs.Element)
    mass_count = system.get_component_number(molcpp.ecs.Mass)
    
    print(f"Position components: {pos_count}")
    print(f"Element components: {elem_count}")
    print(f"Mass components: {mass_count}")
    
    # Find entities with specific components
    entities_with_element = system.get_entities_with_component(molcpp.ecs.Element)
    print(f"Entities with Element component: {len(entities_with_element)}")
    
    for entity_id in entities_with_element:
        print(f"  Entity ID: {entity_id}")


def main():
    """Main example"""
    print("=== molcpp Python Bindings Example ===")
    
    # Create a water molecule
    atoms = create_water_molecule()
    
    # Analyze the system
    analyze_system()
    
    print("\n✓ Example completed successfully!")


if __name__ == "__main__":
    main()
