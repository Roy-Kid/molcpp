"""
Test ECS system functionality using pytest
"""

import pytest
import molcpp


class TestECSSystem:
    """Test cases for ECS system components and entities"""
    
    def test_entity_creation_and_ids(self):
        """Test entity creation and unique ID assignment"""
        entity1 = molcpp.ecs.Entity()
        entity2 = molcpp.ecs.Entity()
        
        # Each entity should have a unique ID
        assert isinstance(entity1.get_id(), int)
        assert isinstance(entity2.get_id(), int)
        assert entity1.get_id() != entity2.get_id()
    
    def test_entity_component_management(self):
        """Test basic entity component operations"""
        entity = molcpp.ecs.Entity()
        
        # Initially no components
        assert not entity.has_component(molcpp.ecs.Position)
        assert not entity.has_component(molcpp.ecs.Element)
        
        # Add Position component
        pos = entity.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        assert entity.has_component(molcpp.ecs.Position)
        assert pos.x == 1.0
        assert pos.y == 2.0
        assert pos.z == 3.0
        
        # Get component
        retrieved_pos = entity.get_component(molcpp.ecs.Position)
        assert retrieved_pos is not None
        assert retrieved_pos.x == 1.0
        
        # Remove component
        removed = entity.remove_component(molcpp.ecs.Position)
        assert removed is True
        assert not entity.has_component(molcpp.ecs.Position)
        
        # Remove non-existent component
        removed_again = entity.remove_component(molcpp.ecs.Position)
        assert removed_again is False
    
    def test_multiple_components_on_entity(self):
        """Test entity with multiple different components"""
        entity = molcpp.ecs.Entity()
        
        # Add multiple components
        pos = entity.add_component(molcpp.ecs.Position, 5.0, 6.0, 7.0)
        elem = entity.add_component(molcpp.ecs.Element, "C", 6)
        mass = entity.add_component(molcpp.ecs.Mass, 12.01)
        vel = entity.add_component(molcpp.ecs.Velocity, 0.1, 0.2, 0.3)
        
        # Verify all components exist
        assert entity.has_component(molcpp.ecs.Position)
        assert entity.has_component(molcpp.ecs.Element)
        assert entity.has_component(molcpp.ecs.Mass)
        assert entity.has_component(molcpp.ecs.Velocity)
        
        # Verify component data
        assert pos.x == 5.0 and pos.y == 6.0 and pos.z == 7.0
        assert elem.symbol == "C" and elem.atomic_number == 6
        assert mass.value == 12.01
        assert vel.vx == 0.1 and vel.vy == 0.2 and vel.vz == 0.3
        
        # Remove one component, others should remain
        entity.remove_component(molcpp.ecs.Position)
        assert not entity.has_component(molcpp.ecs.Position)
        assert entity.has_component(molcpp.ecs.Element)
        assert entity.has_component(molcpp.ecs.Mass)
        assert entity.has_component(molcpp.ecs.Velocity)
    
    def test_component_default_constructors(self):
        """Test components created with default constructors"""
        entity = molcpp.ecs.Entity()
        
        # Add components with default constructors
        pos = entity.add_component(molcpp.ecs.Position)
        elem = entity.add_component(molcpp.ecs.Element)
        mass = entity.add_component(molcpp.ecs.Mass)
        radius = entity.add_component(molcpp.ecs.Radius)
        vel = entity.add_component(molcpp.ecs.Velocity)
        charge = entity.add_component(molcpp.ecs.Charge)
        bond_info = entity.add_component(molcpp.ecs.BondInfo)
        
        # Verify default values
        assert pos.x == 0.0 and pos.y == 0.0 and pos.z == 0.0
        assert elem.symbol == "" and elem.atomic_number == 0
        assert mass.value == 0.0
        assert radius.value == 0.0
        assert vel.vx == 0.0 and vel.vy == 0.0 and vel.vz == 0.0
        assert charge.value == 0.0
        assert bond_info.atom1_id == 0 and bond_info.atom2_id == 0
    
    def test_component_modification(self):
        """Test modifying component data after creation"""
        entity = molcpp.ecs.Entity()
        
        # Add component with initial values
        pos = entity.add_component(molcpp.ecs.Position, 1.0, 2.0, 3.0)
        assert pos.x == 1.0
        
        # Modify component
        pos.x = 10.0
        pos.y = 20.0
        pos.z = 30.0
        
        # Verify modification persisted
        retrieved_pos = entity.get_component(molcpp.ecs.Position)
        assert retrieved_pos.x == 10.0
        assert retrieved_pos.y == 20.0
        assert retrieved_pos.z == 30.0
    
    def test_entity_repr(self):
        """Test entity string representation"""
        entity = molcpp.ecs.Entity()
        repr_str = repr(entity)
        assert "Entity" in repr_str
        assert str(entity.get_id()) in repr_str


class TestComponents:
    """Test cases for individual component classes"""
    
    def test_position_component(self):
        """Test Position component"""
        # Default constructor
        pos1 = molcpp.ecs.Position()
        assert pos1.x == 0.0 and pos1.y == 0.0 and pos1.z == 0.0
        
        # Parameterized constructor
        pos2 = molcpp.ecs.Position(1.5, 2.5, 3.5)
        assert pos2.x == 1.5 and pos2.y == 2.5 and pos2.z == 3.5
        
        # Modification
        pos2.x = 10.0
        assert pos2.x == 10.0
        
        # String representation
        repr_str = repr(pos2)
        assert "Position" in repr_str
        assert "10" in repr_str
    
    def test_element_component(self):
        """Test Element component"""
        # Default constructor
        elem1 = molcpp.ecs.Element()
        assert elem1.symbol == "" and elem1.atomic_number == 0
        
        # Parameterized constructor
        elem2 = molcpp.ecs.Element("H", 1)
        assert elem2.symbol == "H" and elem2.atomic_number == 1
        
        # Modification
        elem2.symbol = "He"
        elem2.atomic_number = 2
        assert elem2.symbol == "He" and elem2.atomic_number == 2
        
        # String representation
        repr_str = repr(elem2)
        assert "Element" in repr_str
        assert "He" in repr_str
    
    def test_mass_component(self):
        """Test Mass component"""
        # Default constructor
        mass1 = molcpp.ecs.Mass()
        assert mass1.value == 0.0
        
        # Parameterized constructor
        mass2 = molcpp.ecs.Mass(12.01)
        assert mass2.value == 12.01
        
        # Modification
        mass2.value = 14.007
        assert mass2.value == 14.007
        
        # String representation
        repr_str = repr(mass2)
        assert "Mass" in repr_str
        assert "14.007" in repr_str
    
    def test_radius_component(self):
        """Test Radius component"""
        # Default constructor
        radius1 = molcpp.ecs.Radius()
        assert radius1.value == 0.0
        
        # Parameterized constructor
        radius2 = molcpp.ecs.Radius(1.2)
        assert radius2.value == 1.2
        
        # Modification
        radius2.value = 1.5
        assert radius2.value == 1.5
        
        # String representation
        repr_str = repr(radius2)
        assert "Radius" in repr_str
        assert "1.5" in repr_str
    
    def test_velocity_component(self):
        """Test Velocity component"""
        # Default constructor
        vel1 = molcpp.ecs.Velocity()
        assert vel1.vx == 0.0 and vel1.vy == 0.0 and vel1.vz == 0.0
        
        # Parameterized constructor
        vel2 = molcpp.ecs.Velocity(1.0, 2.0, 3.0)
        assert vel2.vx == 1.0 and vel2.vy == 2.0 and vel2.vz == 3.0
        
        # Modification
        vel2.vx = 10.0
        assert vel2.vx == 10.0
        
        # String representation
        repr_str = repr(vel2)
        assert "Velocity" in repr_str
        assert "10" in repr_str
    
    def test_charge_component(self):
        """Test Charge component"""
        # Default constructor
        charge1 = molcpp.ecs.Charge()
        assert charge1.value == 0.0
        
        # Parameterized constructor
        charge2 = molcpp.ecs.Charge(-1.0)
        assert charge2.value == -1.0
        
        # Modification
        charge2.value = 2.0
        assert charge2.value == 2.0
        
        # String representation
        repr_str = repr(charge2)
        assert "Charge" in repr_str
        assert "2" in repr_str
    
    def test_bond_info_component(self):
        """Test BondInfo component"""
        # Default constructor
        bond1 = molcpp.ecs.BondInfo()
        assert bond1.atom1_id == 0 and bond1.atom2_id == 0
        
        # Parameterized constructor
        bond2 = molcpp.ecs.BondInfo(10, 20)
        assert bond2.atom1_id == 10 and bond2.atom2_id == 20
        
        # Modification
        bond2.atom1_id = 100
        bond2.atom2_id = 200
        assert bond2.atom1_id == 100 and bond2.atom2_id == 200
        
        # String representation
        repr_str = repr(bond2)
        assert "BondInfo" in repr_str
        assert "100" in repr_str
        assert "200" in repr_str
    
    def test_component_type_information(self):
        """Test component type information methods"""
        pos = molcpp.ecs.Position()
        elem = molcpp.ecs.Element()
        mass = molcpp.ecs.Mass()
        
        # Test get_type_name method
        assert pos.get_type_name() == "Position"
        assert elem.get_type_name() == "Element"
        assert mass.get_type_name() == "Mass"
        
        # Different component types should have different type names
        assert pos.get_type_name() != elem.get_type_name()
        assert elem.get_type_name() != mass.get_type_name()
        assert pos.get_type_name() != mass.get_type_name()
