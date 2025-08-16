"""
Test Bond functionality using pytest
"""

import pytest
import molcpp


class TestBond:
    """Test cases for Bond class"""
    
    def test_bond_creation(self):
        """Test Bond creation and inheritance from Entity"""
        bond = molcpp.bond.Bond()
        
        # Bond should have unique ID (inherited from Entity)
        assert isinstance(bond.get_id(), int)
        
        # Multiple bonds should have different IDs
        bond2 = molcpp.bond.Bond()
        assert bond.get_id() != bond2.get_id()
    
    def test_bond_creation_with_atoms(self):
        """Test Bond creation with atom references"""
        atom1 = molcpp.atom.Atom()
        atom2 = molcpp.atom.Atom()
        
        # Create bond with atom references
        bond = molcpp.bond.Bond(atom1, atom2)
        
        # Should have BondInfo component with correct atom IDs
        assert bond.has_component(molcpp.ecs.BondInfo)
        bond_info = bond.get_component(molcpp.ecs.BondInfo)
        assert bond_info.atom1_id == atom1.get_id()
        assert bond_info.atom2_id == atom2.get_id()
    
    def test_bond_inherits_entity_methods(self, clean_bond):
        """Test that Bond inherits all Entity component management methods"""
        # Test add_component (inherited from Entity)
        pos_comp = clean_bond.add_component(molcpp.ecs.Position, 0.5, 1.0, 1.5)
        assert pos_comp is not None
        assert pos_comp.x == 0.5
        
        # Test has_component (inherited from Entity)
        assert clean_bond.has_component(molcpp.ecs.Position)
        assert not clean_bond.has_component(molcpp.ecs.Element)
        
        # Test get_component (inherited from Entity)
        retrieved_pos = clean_bond.get_component(molcpp.ecs.Position)
        assert retrieved_pos is not None
        assert retrieved_pos.x == 0.5
        
        # Test remove_component (inherited from Entity)
        removed = clean_bond.remove_component(molcpp.ecs.Position)
        assert removed is True
        assert not clean_bond.has_component(molcpp.ecs.Position)
    
    def test_bond_component_composition(self, clean_bond):
        """Test Bond component composition capabilities"""
        # Add BondInfo component manually
        bond_info = clean_bond.add_component(molcpp.ecs.BondInfo, 1, 2)
        assert bond_info.atom1_id == 1
        assert bond_info.atom2_id == 2
        
        # Add other physics components
        charge_comp = clean_bond.add_component(molcpp.ecs.Charge, -0.5)
        assert charge_comp.value == -0.5
        
        # Verify multiple components coexist
        assert clean_bond.has_component(molcpp.ecs.BondInfo)
        assert clean_bond.has_component(molcpp.ecs.Charge)
        
        # Verify component data integrity
        retrieved_bond_info = clean_bond.get_component(molcpp.ecs.BondInfo)
        assert retrieved_bond_info.atom1_id == 1
        assert retrieved_bond_info.atom2_id == 2
        
        retrieved_charge = clean_bond.get_component(molcpp.ecs.Charge)
        assert retrieved_charge.value == -0.5
    
    def test_bond_repr(self):
        """Test Bond string representation"""
        bond = molcpp.bond.Bond()
        repr_str = repr(bond)
        assert "Bond" in repr_str
        assert str(bond.get_id()) in repr_str
    
    def test_bond_with_invalid_components(self, clean_bond):
        """Test Bond behavior with invalid component operations"""
        # Test getting non-existent component
        non_existent = clean_bond.get_component(molcpp.ecs.Element)
        assert non_existent is None
        
        # Test removing non-existent component
        removed = clean_bond.remove_component(molcpp.ecs.Element)
        assert removed is False
        
        # Test has_component for non-existent component
        assert not clean_bond.has_component(molcpp.ecs.Element)
    
    def test_bond_component_lifecycle(self, clean_bond):
        """Test complete lifecycle of bond components"""
        # Start with no components
        assert not clean_bond.has_component(molcpp.ecs.BondInfo)
        
        # Add BondInfo component
        bond_info = clean_bond.add_component(molcpp.ecs.BondInfo, 10, 20)
        assert clean_bond.has_component(molcpp.ecs.BondInfo)
        assert bond_info.atom1_id == 10
        assert bond_info.atom2_id == 20
        
        # Modify component data
        bond_info.atom1_id = 100
        bond_info.atom2_id = 200
        
        # Verify modification persisted
        retrieved = clean_bond.get_component(molcpp.ecs.BondInfo)
        assert retrieved.atom1_id == 100
        assert retrieved.atom2_id == 200
        
        # Remove component
        removed = clean_bond.remove_component(molcpp.ecs.BondInfo)
        assert removed is True
        assert not clean_bond.has_component(molcpp.ecs.BondInfo)
        
        # Verify component is gone
        none_comp = clean_bond.get_component(molcpp.ecs.BondInfo)
        assert none_comp is None
    
    def test_bond_multiple_component_types(self, clean_bond):
        """Test Bond with multiple different component types"""
        # Add various components
        position = clean_bond.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        velocity = clean_bond.add_component(molcpp.ecs.Velocity, 0.1, 0.2, 0.3)
        charge = clean_bond.add_component(molcpp.ecs.Charge, 1.5)
        bond_info = clean_bond.add_component(molcpp.ecs.BondInfo, 5, 6)
        
        # Verify all components exist
        assert clean_bond.has_component(molcpp.ecs.Position)
        assert clean_bond.has_component(molcpp.ecs.Velocity)
        assert clean_bond.has_component(molcpp.ecs.Charge)
        assert clean_bond.has_component(molcpp.ecs.BondInfo)
        
        # Verify component data
        assert position.x == 1.0 and position.y == 2.0 and position.z == 3.0
        assert velocity.vx == 0.1 and velocity.vy == 0.2 and velocity.vz == 0.3
        assert charge.value == 1.5
        assert bond_info.atom1_id == 5 and bond_info.atom2_id == 6
        
        # Remove one component, verify others remain
        clean_bond.remove_component(molcpp.ecs.Position)
        assert not clean_bond.has_component(molcpp.ecs.Position)
        assert clean_bond.has_component(molcpp.ecs.Velocity)
        assert clean_bond.has_component(molcpp.ecs.Charge)
        assert clean_bond.has_component(molcpp.ecs.BondInfo)
