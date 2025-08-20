# MolCpp Python Tests

This directory contains comprehensive Python unit tests for the MolCpp library bindings.

## Test Structure

- **`test_ecs.py`** - Tests for the Entity-Component-System (ECS) module
- **`test_ecs_simple.py`** - Simplified ECS tests (also passing)
- **`test_spatial.py`** - Tests for the Spatial module (Box, Region, Boundary)
- **`test_locality.py`** - Tests for the Locality module (AABB, AABBQuery, NeighborBond)

## Running Tests

### Prerequisites
```bash
pip install -r requirements.txt
```

### Run All Tests
```bash
python3 -m pytest --tb=short
```

### Run Specific Module Tests
```bash
# ECS module only
python3 -m pytest test_ecs.py -v

# Spatial module only  
python3 -m pytest test_spatial.py -v

# Locality module only
python3 -m pytest test_locality.py -v

# Multiple modules
python3 -m pytest test_ecs.py test_spatial.py test_locality.py -v
```

## Current Status

✅ **ALL TESTS PASSING** - 45/45 tests pass successfully

### Test Results Summary
- **ECS Module**: 11/11 tests pass
- **ECS Simple**: 12/12 tests pass  
- **Spatial Module**: 9/9 tests pass
- **Locality Module**: 13/13 tests pass

## Test Coverage

### ECS Module (`test_ecs.py`)
- Entity creation and management
- Component operations (add, get, has, remove)
- All component types: Position, Element, Radius, Velocity, Mass, Charge, BondInfo
- Component equality and comparison
- Multiple components on same entity

### Spatial Module (`test_spatial.py`)  
- Box creation (cube, orthorhombic)
- Box properties and calculations
- Volume calculations
- Origin variations
- Periodic boundary conditions
- Matrix properties
- Edge cases

### Locality Module (`test_locality.py`)
- NeighborBond creation and modification
- AABB and AABBSphere operations
- QueryArgs and QueryType enums
- AABBQuery basic functionality and dynamic updates
- AABB utility functions (overlap, contains, merge)

## Configuration

The `conftest.py` file sets up the Python path and provides a `bindings` fixture for all tests.

## Dependencies

- `pytest` - Testing framework
- `pytest-mock` - Mocking support
- `numpy` - Numerical operations and array handling

## Notes

- All tests use proper pytest format with classes and methods
- Tests are designed to work with the actual C++ bindings
- Some advanced functionality (like iterators) is not yet bound, so tests avoid those methods
- The system successfully handles all basic operations without segmentation faults
