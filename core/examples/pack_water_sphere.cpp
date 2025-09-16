// Minimal example: pack water molecules inside a spherical region
// This is a header-only usage example; no build integration required.

#include <iostream>
#include <random>
#include <xtensor/xarray.hpp>
#include "../include/molcpp/core/frame.hpp"
#include "../include/molcpp/pack/packer.hpp"
#include "../include/molcpp/pack/constraint.hpp"

using molcpp::Frame;
using molcpp::Block;
using molcpp::pack::MolPacker;
using molcpp::pack::Constraint;
using molcpp::pack::make_inside_sphere;
using molcpp::pack::make_inter_molecular_min_distance;

// Create a single water molecule geometry (TIP3P-like distances, arbitrary orientation)
// Units: Angstrom
static Frame make_water_frame() {
    Frame f;
    Block atoms;
    // O at origin, H at ~0.9572 A at 104.52 deg
    // Simple coordinates (approx):
    // O (0, 0, 0)
    // H1 (0.9572, 0, 0)
    // H2 (-0.2399872, 0.927297, 0)
    xt::xarray<float> coords{{0.0f, 0.0f, 0.0f},
                             {0.9572f, 0.0f, 0.0f},
                             {-0.2399872f, 0.927297f, 0.0f}};
    atoms["coords"] = coords;

    // Optional: atom ids and types
    xt::xarray<int> ids{1, 2, 3};
    xt::xarray<int> types{8, 1, 1}; // O=8, H=1 (arbitrary)
    atoms["id"] = ids;
    atoms["type"] = types;
    f["atoms"] = atoms;
    return f;
}

int main() {
    // Build a water molecule frame
    Frame water = make_water_frame();

    // Create a packer
    MolPacker packer;

    // Define constraints: inside sphere radius R, center at (0,0,0)
    float radius = 15.0f;
    Constraint inside = make_inside_sphere(radius);

    // Intermolecular minimum distance (only between different water molecules)
    const std::size_t atoms_per_water = 3; // O H H
    float min_inter = 2.2f; // Hard floor; increase to make it sparser
    Constraint inter = make_inter_molecular_min_distance(min_inter, atoms_per_water);

    // Combine constraints: inside sphere AND inter-molecular hard minimum distance
    Constraint combined = inside & inter;

    // Define a target: N water molecules under inside-sphere constraint
    size_t n_waters = 200;
    packer.def_target(water, n_waters, combined, /*is_fixed=*/false, "WAT");

    // Run packing with basic parameters (placeholder optimizer currently)
    auto result = packer.pack(/*targets=*/{}, /*max_steps=*/5000, /*seed=*/1234);

    // Print summary
    const auto& atoms = result["atoms"];
    auto coords = atoms["coords"];
    std::cout << "Packed atoms: " << coords.shape(0) << " in sphere R=" << radius << "\n";

    // Output first 5 coordinates
    size_t n = std::min<size_t>(coords.shape(0), 5);
    for (size_t i = 0; i < n; ++i) {
        std::cout << i+1 << ": "
                  << static_cast<double>(coords(i,0)) << " "
                  << static_cast<double>(coords(i,1)) << " "
                  << static_cast<double>(coords(i,2)) << "\n";
    }

    return 0;
}
