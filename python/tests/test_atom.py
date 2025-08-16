"""
Test Atom functionality using pytest
"""

import pytest
import molcpp


class TestAtom:
    """Test cases for Atom class"""
    
    def test_atom_creation(self):
        """Test Atom creation and inheritance from Entity"""
        atom = molcpp.atom.Atom()
        
        # Atom should have unique ID (inherited from Entity)
        assert isinstance(atom.get_id(), int)
        
        # Multiple atoms should have different IDs
        atom2 = molcpp.atom.Atom()
        assert atom.get_id() != atom2.get_id()
    
    def test_atom_inherits_entity_methods(self, clean_atom):
        """Test that Atom inherits all Entity component management methods"""
        # Test add_component (inherited from Entity)
        pos_comp = clean_atom.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        assert pos_comp is not None
        assert pos_comp.x == 1.0
        
        # Test has_component (inherited from Entity)
        assert clean_atom.has_component(molcpp.ecs.Position)
        assert not clean_atom.has_component(molcpp.ecs.Element)
        
        # Test get_component (inherited from Entity)
        retrieved_pos = clean_atom.get_component(molcpp.ecs.Position)
        assert retrieved_pos is not None
        assert retrieved_pos.x == 1.0
        
        # Test remove_component (inherited from Entity)
        removed = clean_atom.remove_component(molcpp.ecs.Position)
        assert removed is True
        assert not clean_atom.has_component(molcpp.ecs.Position)
    
    def test_set_position_convenience_method(self, clean_atom):
        """Test Atom's set_position convenience method"""
        # Set position using convenience method
        clean_atom.set_position(5.0, 6.0, 7.0)
        
        # Verify position was set correctly
        pos_comp = clean_atom.get_component(molcpp.ecs.Position)
        assert pos_comp is not None
        assert pos_comp.x == 5.0
        assert pos_comp.y == 6.0
        assert pos_comp.z == 7.0
    
    def test_set_element_convenience_method(self, clean_atom):
        """Test Atom's set_element convenience method"""
        # Set element using convenience method
        clean_atom.set_element("N", 7)
        
        # Verify element was set correctly
        elem_comp = clean_atom.get_component(molcpp.ecs.Element)
        assert elem_comp is not None
        assert elem_comp.symbol == "N"
        assert elem_comp.atomic_number == 7
    
    def test_atom_component_composition(self, clean_atom):
        """Test Atom component composition capabilities"""
        # Add multiple components
        pos = clean_atom.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        elem = clean_atom.add_component(molcpp.ecs.Element, "C", 6)
        mass = clean_atom.add_component(molcpp.ecs.Mass, 12.01)
        radius = clean_atom.add_component(molcpp.ecs.Radius, 1.2)
        vel = clean_atom.add_component(molcpp.ecs.Velocity, 0.1, 0.2, 0.3)
        charge = clean_atom.add_component(molcpp.ecs.Charge, -1.0)
        
        # Verify all components exist
        assert clean_atom.has_component(molcpp.ecs.Position)
        assert clean_atom.has_component(molcpp.ecs.Element)
        assert clean_atom.has_component(molcpp.ecs.Mass)
        assert clean_atom.has_component(molcpp.ecs.Radius)
        assert clean_atom.has_component(molcpp.ecs.Velocity)
        assert clean_atom.has_component(molcpp.ecs.Charge)
        
        # Verify component data integrity
        assert pos.x == 1.0 and pos.y == 2.0 and pos.z == 3.0
        assert elem.symbol == "C" and elem.atomic_number == 6
        assert mass.value == 12.01
        assert radius.value == 1.2
        assert vel.vx == 0.1 and vel.vy == 0.2 and vel.vz == 0.3
        assert charge.value == -1.0
    
    def test_atom_repr(self):
        """Test Atom string representation"""
        atom = molcpp.atom.Atom()
        repr_str = repr(atom)
        assert "Atom" in repr_str
        assert str(atom.get_id()) in repr_str
    
    def test_atom_with_invalid_components(self, clean_atom):
        """Test Atom behavior with invalid component operations"""
        # Test getting non-existent component
        non_existent = clean_atom.get_component(molcpp.ecs.BondInfo)
        assert non_existent is None
        
        # Test removing non-existent component
        removed = clean_atom.remove_component(molcpp.ecs.BondInfo)
        assert removed is False
        
        # Test has_component for non-existent component
        assert not clean_atom.has_component(molcpp.ecs.BondInfo)
    
    def test_atom_component_lifecycle(self, clean_atom):
        """Test complete lifecycle of atom components"""
        # Start with no Position component
        assert not clean_atom.has_component(molcpp.ecs.Position)
        
        # Add Position component
        pos = clean_atom.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        assert clean_atom.has_component(molcpp.ecs.Position)
        assert pos.x == 1.0
        
        # Modify component data
        pos.x = 10.0
        pos.y = 20.0
        pos.z = 30.0
        
        # Verify modification persisted
        retrieved = clean_atom.get_component(molcpp.ecs.Position)
        assert retrieved.x == 10.0
        assert retrieved.y == 20.0
        assert retrieved.z == 30.0
        
        # Remove component
        removed = clean_atom.remove_component(molcpp.ecs.Position)
        assert removed is True
        assert not clean_atom.has_component(molcpp.ecs.Position)
        
        # Verify component is gone
        none_comp = clean_atom.get_component(molcpp.ecs.Position)
        assert none_comp is None
    
    def test_atom_physics_properties(self, clean_atom):
        """Test Atom with physics-related components"""
        # Add physics components
        mass = clean_atom.add_component(molcpp.ecs.Mass, 1.008)  # Hydrogen mass
        radius = clean_atom.add_component(molcpp.ecs.Radius, 0.37)  # Hydrogen radius
        velocity = clean_atom.add_component(molcpp.ecs.Velocity, 100.0, 200.0, 300.0)
        charge = clean_atom.add_component(molcpp.ecs.Charge, 1.0)  # Proton charge
        
        # Verify physics properties
        assert mass.value == 1.008
        assert radius.value == 0.37
        assert velocity.vx == 100.0 and velocity.vy == 200.0 and velocity.vz == 300.0
        assert charge.value == 1.0
        
        # Modify physics properties
        velocity.vx = 150.0
        charge.value = 0.0  # Neutral
        
        # Verify modifications
        assert velocity.vx == 150.0
        assert charge.value == 0.0
    
    def test_atom_element_properties(self, clean_atom):
        """Test Atom with different element types"""
        # Test hydrogen
        clean_atom.set_element("H", 1)
        elem = clean_atom.get_component(molcpp.ecs.Element)
        assert elem.symbol == "H" and elem.atomic_number == 1
        
        # Change to carbon
        elem.symbol = "C"
        elem.atomic_number = 6
        retrieved = clean_atom.get_component(molcpp.ecs.Element)
        assert retrieved.symbol == "C" and retrieved.atomic_number == 6
        
        # Change to oxygen
        elem.symbol = "O"
        elem.atomic_number = 8
        assert elem.symbol == "O" and elem.atomic_number == 8
    
    def test_atom_complex_scenario(self, clean_atom):
        """Test complex atom setup with multiple operations"""
        # Create a carbon atom with full properties
        clean_atom.set_element("C", 6)
        clean_atom.set_position(0.0, 0.0, 0.0)
        
        mass = clean_atom.add_component(molcpp.ecs.Mass, 12.01)
        radius = clean_atom.add_component(molcpp.ecs.Radius, 0.77)
        velocity = clean_atom.add_component(molcpp.ecs.Velocity, 0.0, 0.0, 0.0)
        charge = clean_atom.add_component(molcpp.ecs.Charge, 0.0)
        
        # Verify complete atom setup
        assert clean_atom.has_component(molcpp.ecs.Element)
        assert clean_atom.has_component(molcpp.ecs.Position)
        assert clean_atom.has_component(molcpp.ecs.Mass)
        assert clean_atom.has_component(molcpp.ecs.Radius)
        assert clean_atom.has_component(molcpp.ecs.Velocity)
        assert clean_atom.has_component(molcpp.ecs.Charge)
        
        # Verify data integrity
        elem = clean_atom.get_component(molcpp.ecs.Element)
        pos = clean_atom.get_component(molcpp.ecs.Position)
        
        assert elem.symbol == "C" and elem.atomic_number == 6
        assert pos.x == 0.0 and pos.y == 0.0 and pos.z == 0.0
        assert mass.value == 12.01
        assert radius.value == 0.77
        assert velocity.vx == 0.0 and velocity.vy == 0.0 and velocity.vz == 0.0
        assert charge.value == 0.0
        
        # Simulate movement
        pos.x = 1.0
        velocity.vx = 10.0
        
        # Verify changes
        assert pos.x == 1.0
        assert velocity.vx == 10.0
