# MolPack: Molecular Packing Library

MolPack is a native C++ implementation of molecular packing algorithms, designed to be compatible with the Python MolPy implementation while providing high-performance C++ implementations.

## Overview

MolPack provides a constraint-based approach to molecular packing:
- **Constraints**: Define spatial regions and conditions for molecular placement
- **Targets**: Specify molecular structures and their packing requirements
- **Packer**: Core algorithm that optimizes molecular positions

## Key Components

### 1. Constraints

Constraints define spatial regions and provide penalty functions for optimization:

```cpp
#include <molcpp/pack/constraint.hpp>

// Box constraints
auto inside_box = std::make_unique<InsideBoxConstraint<double>>(
    Vec3<double>{10.0, 10.0, 10.0},  // lengths
    Vec3<double>{0.0, 0.0, 0.0}       // origin
);

auto outside_box = std::make_unique<OutsideBoxConstraint<double>>(
    Vec3<double>{0.0, 0.0, 0.0},      // origin
    Vec3<double>{5.0, 5.0, 5.0}       // lengths
);

// Sphere constraints
auto inside_sphere = std::make_unique<InsideSphereConstraint<double>>(
    5.0,                               // radius
    Vec3<double>{0.0, 0.0, 0.0}       // center
);

// Distance constraints
auto min_distance = std::make_unique<MinDistanceConstraint<double>>(2.0);

// Composite constraints
auto complex_constraint = (*inside_box) & (*outside_box) | (*inside_sphere);
```

### 2. Targets

Targets represent molecular structures to be packed:

```cpp
#include <molcpp/pack/target.hpp>

// Create a target
auto target = Target<double>(
    molecular_frame,                    // Frame with molecular structure
    5,                                 // Number of copies to pack
    std::move(constraint),             // Spatial constraint
    false,                             // Not fixed
    nullptr,                           // No custom optimizer
    "water_molecules"                  // Name
);
```

### 3. MolPacker

The core packing algorithm:

```cpp
#include <molcpp/pack/packer.hpp>

// Create packer
MolPacker<double> packer;

// Add targets
packer.add_target(target1);
packer.add_target(target2);

// Or define targets directly
packer.def_target(frame, 10, std::move(constraint), false, "methane");

// Pack molecules
auto result = packer.pack({}, 1000, 42);  // max_steps=1000, seed=42
```

## Complete Example

```cpp
#include <molcpp/pack/packer.hpp>
#include <molcpp/pack/constraint.hpp>

int main() {
    // Create constraints
    auto box_constraint = std::make_unique<InsideBoxConstraint<double>>(
        Vec3<double>{20.0, 20.0, 20.0}
    );
    
    auto distance_constraint = std::make_unique<MinDistanceConstraint<double>>(3.0);
    
    // Combine constraints
    auto total_constraint = (*box_constraint) & (*distance_constraint);
    
    // Create packer
    MolPacker<double> packer;
    
    // Add water molecules
    packer.def_target(water_frame, 100, std::move(total_constraint), false, "water");
    
    // Pack molecules
    auto packed_system = packer.pack({}, 2000, 12345);
    
    return 0;
}
```

## Constraint Types

### Geometric Constraints

- **InsideBoxConstraint**: Points must be inside a box
- **OutsideBoxConstraint**: Points must be outside a box
- **InsideSphereConstraint**: Points must be inside a sphere
- **OutsideSphereConstraint**: Points must be outside a sphere

### Distance Constraints

- **MinDistanceConstraint**: Enforces minimum distance between all points

### Composite Constraints

- **AndConstraint**: Both constraints must be satisfied (logical AND)
- **OrConstraint**: At least one constraint must be satisfied (logical OR)

## Optimization Algorithm

MolPacker uses a gradient-based optimization approach:

1. **Initialization**: Generate random initial positions
2. **Constraint Evaluation**: Calculate penalty values and gradients
3. **Gradient Descent**: Update positions using constraint gradients
4. **Convergence**: Check for convergence or maximum iterations
5. **Result Construction**: Build final Frame with optimized positions

## Performance Features

- **Template-based**: Support for float and double precision
- **Memory Efficient**: Uses xtensor for optimized array operations
- **Gradient-based**: Fast convergence using constraint gradients
- **Native C++**: No external dependencies or file I/O overhead

## Integration with MolPy

When binding to Python:
- Constraints provide the same penalty() and dpenalty() interface
- Targets maintain the same structure and behavior
- MolPacker replaces the external packmol dependency
- Results are compatible with MolPy Frame objects

## Future Enhancements

- **Advanced Optimizers**: Support for different optimization algorithms
- **Parallel Processing**: Multi-threaded constraint evaluation
- **GPU Acceleration**: CUDA/OpenCL support for large systems
- **Additional Constraints**: More complex geometric and chemical constraints

## Dependencies

- **xtensor**: For efficient numerical arrays
- **C++17**: For modern C++ features
- **Standard Library**: For containers and algorithms

