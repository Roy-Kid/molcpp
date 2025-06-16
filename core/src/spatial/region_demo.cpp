#include "molcpp/spatial/region.hpp"
#include "molcpp/spatial/boundary.hpp"
#include <xtensor/xarray.hpp>
#include <xtensor/xbuilder.hpp>
#include <xtensor/xrandom.hpp>
#include <iostream>
#include <memory>
#include <iomanip>

using namespace molcpp;

void demonstrate_basic_regions() {
    std::cout << "=== Basic Region Demonstration ===" << std::endl;
    
    // Create different types of regions
    InsideCube cube({0.0, 0.0, 0.0}, 2.0);
    InsideSphere sphere({1.0, 1.0, 1.0}, 1.0);
    InsideCylinder cylinder({0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}, 0.5);
    NearPlane plane({1.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, 0.3);
    
    // Test coordinates
    xt::xarray<double> test_coords = {
        {0.5, 0.5, 0.5},   // Inside most regions
        {1.5, 1.5, 1.5},   // Inside some regions
        {3.0, 3.0, 3.0}    // Outside most regions
    };
    
    std::cout << "Test coordinates:" << std::endl;
    for (size_t i = 0; i < test_coords.shape(0); ++i) {
        std::cout << "  Point " << i << ": (" 
                  << test_coords(i, 0) << ", " 
                  << test_coords(i, 1) << ", " 
                  << test_coords(i, 2) << ")" << std::endl;
    }
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nRegion containment tests (all points must be inside):" << std::endl;
    std::cout << "Cube [0,2]³:        " << (cube.isin(test_coords) ? "PASS" : "FAIL") 
              << " (volume: " << cube.volume() << ")" << std::endl;
    std::cout << "Sphere center(1,1,1) R=1: " << (sphere.isin(test_coords) ? "PASS" : "FAIL")
              << " (volume: " << sphere.volume() << ")" << std::endl;
    std::cout << "Cylinder Z-axis R=0.5:   " << (cylinder.isin(test_coords) ? "PASS" : "FAIL")
              << " (volume: " << cylinder.volume() << ")" << std::endl;
    std::cout << "Plane Z=0 ±0.3:          " << (plane.isin(test_coords) ? "PASS" : "FAIL") << std::endl;
    
    // Test individual points with mask
    std::cout << "\nIndividual point analysis:" << std::endl;
    auto cube_mask = cube.mask(test_coords);
    auto sphere_mask = sphere.mask(test_coords);
    auto cylinder_mask = cylinder.mask(test_coords);
    auto plane_mask = plane.mask(test_coords);
    
    for (size_t i = 0; i < test_coords.shape(0); ++i) {
        std::cout << "Point " << i << ": ";
        std::cout << "Cube:" << (cube_mask(i) ? "✓" : "✗") << " ";
        std::cout << "Sphere:" << (sphere_mask(i) ? "✓" : "✗") << " ";
        std::cout << "Cylinder:" << (cylinder_mask(i) ? "✓" : "✗") << " ";
        std::cout << "Plane:" << (plane_mask(i) ? "✓" : "✗") << std::endl;
    }
}

void demonstrate_boolean_combinations() {
    std::cout << "\n=== Boolean Region Combinations ===" << std::endl;
    
    // Create basic regions
    auto cube = std::make_shared<InsideCube>(Vec3{-1.0, -1.0, -1.0}, 2.0);  // [-1,1]³
    auto sphere = std::make_shared<InsideSphere>(Vec3{0.0, 0.0, 0.0}, 1.0); // Unit sphere
    auto cylinder = std::make_shared<InsideCylinder>(Vec3{-1.0, 0.0, -1.0}, Vec3{1.0, 0.0, 1.0}, 0.6);
    
    // Create boolean combinations
    AndRegion intersection(cube, sphere);          // Cube AND Sphere
    OrRegion union_region(sphere, cylinder);       // Sphere OR Cylinder  
    NotRegion complement(sphere);                  // NOT Sphere
    
    // Complex combination: (Cube AND Sphere) OR (NOT Cylinder)
    auto intersection_ptr = std::make_shared<AndRegion>(cube, sphere);
    auto not_cylinder = std::make_shared<NotRegion>(cylinder);
    OrRegion complex_region(intersection_ptr, not_cylinder);
    
    // Test points
    xt::xarray<double> test_points = {
        {0.5, 0.0, 0.0},    // Inside sphere and cube
        {0.9, 0.9, 0.9},    // Inside cube, outside sphere
        {0.0, 0.0, 0.0},    // Origin - center of sphere
        {2.0, 2.0, 2.0}     // Far outside everything
    };
    
    std::cout << "Boolean region tests:" << std::endl;
    std::cout << "Regions: Cube∩Sphere | Sphere∪Cylinder | ¬Sphere | Complex" << std::endl;
    
    auto inter_mask = intersection.mask(test_points);
    auto union_mask = union_region.mask(test_points);
    auto comp_mask = complement.mask(test_points);
    auto complex_mask = complex_region.mask(test_points);
    
    for (size_t i = 0; i < test_points.shape(0); ++i) {
        std::cout << "Point (" << test_points(i, 0) << "," << test_points(i, 1) 
                  << "," << test_points(i, 2) << "): ";
        std::cout << (inter_mask(i) ? "✓" : "✗") << "            ";
        std::cout << (union_mask(i) ? "✓" : "✗") << "              ";
        std::cout << (comp_mask(i) ? "✓" : "✗") << "        ";
        std::cout << (complex_mask(i) ? "✓" : "✗") << std::endl;
    }
    
    // Show boundary information
    std::cout << "\nBoundary information:" << std::endl;
    auto print_boundary = [](const std::string& name, const std::array<double, 6>& bounds) {
        std::cout << name << ": X[" << bounds[0] << "," << bounds[1] << "] "
                  << "Y[" << bounds[2] << "," << bounds[3] << "] "
                  << "Z[" << bounds[4] << "," << bounds[5] << "]" << std::endl;
    };
    
    print_boundary("Intersection", intersection.boundary());
    print_boundary("Union       ", union_region.boundary());
}

void demonstrate_boundaries() {
    std::cout << "\n=== Boundary Condition Demonstration ===" << std::endl;
    
    // Create different boundary conditions
    FreeBoundary free_boundary;
    OrthogonalBoundary ortho_boundary({4.0, 4.0, 4.0});
    TriclinicBoundary triclinic_boundary({{2.0, 0.0, 0.0}, {1.0, 2.0, 0.0}, {0.0, 0.0, 3.0}});
    SphericalBoundary sphere_boundary({0.0, 0.0, 0.0}, 2.0, true);
    
    // Test coordinates that need wrapping
    xt::xarray<double> coords_to_wrap = {
        {4.5, 1.0, 1.0},    // Outside ortho box in X
        {1.0, 4.5, 1.0},    // Outside ortho box in Y
        {-0.5, -0.5, 1.0}   // Negative coordinates
    };
    
    std::cout << "Original coordinates:" << std::endl;
    for (size_t i = 0; i < coords_to_wrap.shape(0); ++i) {
        std::cout << "  (" << coords_to_wrap(i, 0) << ", " 
                  << coords_to_wrap(i, 1) << ", " 
                  << coords_to_wrap(i, 2) << ")" << std::endl;
    }
    
    // Test wrapping
    std::cout << "\nAfter wrapping:" << std::endl;
    
    auto free_wrapped = free_boundary.wrap(coords_to_wrap);
    std::cout << "Free boundary (no change):" << std::endl;
    for (size_t i = 0; i < free_wrapped.shape(0); ++i) {
        std::cout << "  (" << free_wrapped(i, 0) << ", " 
                  << free_wrapped(i, 1) << ", " 
                  << free_wrapped(i, 2) << ")" << std::endl;
    }
    
    auto ortho_wrapped = ortho_boundary.wrap(coords_to_wrap);
    std::cout << "Orthogonal boundary:" << std::endl;
    for (size_t i = 0; i < ortho_wrapped.shape(0); ++i) {
        std::cout << "  (" << ortho_wrapped(i, 0) << ", " 
                  << ortho_wrapped(i, 1) << ", " 
                  << ortho_wrapped(i, 2) << ")" << std::endl;
    }
    
    // Test minimum image
    std::cout << "\nMinimum image distances:" << std::endl;
    xt::xarray<double> r1 = {0.1, 0.1, 0.1};
    xt::xarray<double> r2 = {3.9, 0.1, 0.1};
    
    auto dr_free = free_boundary.minimum_image(r1, r2);
    auto dr_ortho = ortho_boundary.minimum_image(r1, r2);
    
    std::cout << "From (0.1,0.1,0.1) to (3.9,0.1,0.1):" << std::endl;
    std::cout << "Free:       dr = (" << dr_free(0) << ", " << dr_free(1) << ", " << dr_free(2) << ")" << std::endl;
    std::cout << "Orthogonal: dr = (" << dr_ortho(0) << ", " << dr_ortho(1) << ", " << dr_ortho(2) << ")" << std::endl;
}

void demonstrate_integration() {
    std::cout << "\n=== Region-Boundary Integration ===" << std::endl;
    
    // Create a region that's larger than the simulation box
    auto large_sphere = std::make_shared<InsideSphere>(Vec3{2.0, 2.0, 2.0}, 3.0);
    
    // Create a periodic boundary
    OrthogonalBoundary boundary({4.0, 4.0, 4.0});
    
    // Generate some particles outside the primary box
    xt::xarray<double> particles = {
        {-0.5, 2.0, 2.0},   // Wraps to (3.5, 2.0, 2.0)
        {4.5, 2.0, 2.0},    // Wraps to (0.5, 2.0, 2.0)
        {2.0, -0.5, 2.0},   // Wraps to (2.0, 3.5, 2.0)
        {2.0, 2.0, 4.5}     // Wraps to (2.0, 2.0, 0.5)
    };
    
    std::cout << "Testing region containment with periodic boundaries:" << std::endl;
    std::cout << "Large sphere: center(2,2,2), radius=3" << std::endl;
    std::cout << "Periodic box: [0,4]³" << std::endl;
    
    // Test before and after wrapping
    std::cout << "\nBefore wrapping:" << std::endl;
    auto mask_before = large_sphere->mask(particles);
    for (size_t i = 0; i < particles.shape(0); ++i) {
        std::cout << "  (" << particles(i, 0) << ", " << particles(i, 1) 
                  << ", " << particles(i, 2) << ") -> " 
                  << (mask_before(i) ? "Inside" : "Outside") << std::endl;
    }
    
    auto wrapped_particles = boundary.wrap(particles);
    std::cout << "\nAfter wrapping:" << std::endl;
    auto mask_after = large_sphere->mask(wrapped_particles);
    for (size_t i = 0; i < wrapped_particles.shape(0); ++i) {
        std::cout << "  (" << wrapped_particles(i, 0) << ", " << wrapped_particles(i, 1) 
                  << ", " << wrapped_particles(i, 2) << ") -> " 
                  << (mask_after(i) ? "Inside" : "Outside") << std::endl;
    }
    
    // Monte Carlo sampling example
    std::cout << "\nMonte Carlo volume estimation:" << std::endl;
    InsideSphere unit_sphere({0.0, 0.0, 0.0}, 1.0);
    
    const size_t n_samples = 10000;
    xt::xarray<double> random_points = xt::random::rand<double>({n_samples, 3}) * 2.0 - 1.0; // [-1,1]³
    
    auto sphere_mask = unit_sphere.mask(random_points);
    size_t count_inside = 0;
    for (size_t i = 0; i < n_samples; ++i) {
        if (sphere_mask(i)) count_inside++;
    }
    
    double estimated_volume = 8.0 * count_inside / n_samples; // Volume of cube is 8
    double exact_volume = unit_sphere.volume();
    
    std::cout << "Unit sphere volume estimation with " << n_samples << " samples:" << std::endl;
    std::cout << "Estimated: " << estimated_volume << std::endl;
    std::cout << "Exact:     " << exact_volume << std::endl;
    std::cout << "Error:     " << std::abs(estimated_volume - exact_volume) << std::endl;
}

void demonstrate_performance() {
    std::cout << "\n=== Performance Demonstration ===" << std::endl;
    
    // Create complex region
    auto cube = std::make_shared<InsideCube>(Vec3{-2.0, -2.0, -2.0}, 4.0);
    auto sphere1 = std::make_shared<InsideSphere>(Vec3{-1.0, 0.0, 0.0}, 1.2);
    auto sphere2 = std::make_shared<InsideSphere>(Vec3{1.0, 0.0, 0.0}, 1.2);
    auto cylinder = std::make_shared<InsideCylinder>(Vec3{0.0, -2.0, 0.0}, Vec3{0.0, 2.0, 0.0}, 0.8);
    
    // Complex region: (Sphere1 OR Sphere2) AND Cube AND (NOT Cylinder)
    auto union_spheres = std::make_shared<OrRegion>(sphere1, sphere2);
    auto not_cylinder = std::make_shared<NotRegion>(cylinder);
    auto temp_and = std::make_shared<AndRegion>(union_spheres, cube);
    AndRegion complex_region(temp_and, not_cylinder);
    
    // Large dataset
    const size_t n_points = 100000;
    auto large_dataset = xt::random::randn<double>({n_points, 3}) * 2.0; // Normal distribution
    
    std::cout << "Testing complex region with " << n_points << " points..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto mask = complex_region.mask(large_dataset);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    size_t count_inside = 0;
    for (size_t i = 0; i < n_points; ++i) {
        if (mask(i)) count_inside++;
    }
    
    std::cout << "Results:" << std::endl;
    std::cout << "Points inside complex region: " << count_inside << " (" 
              << (100.0 * count_inside / n_points) << "%)" << std::endl;
    std::cout << "Processing time: " << duration.count() << " μs" << std::endl;
    std::cout << "Rate: " << (n_points * 1000000.0 / duration.count()) << " points/second" << std::endl;
}

int main() {
    try {
        std::cout << "Spatial Region and Boundary System Demonstration" << std::endl;
        std::cout << "================================================" << std::endl;
        
        demonstrate_basic_regions();
        demonstrate_boolean_combinations();
        demonstrate_boundaries();
        demonstrate_integration();
        demonstrate_performance();
        
        std::cout << "\nDemonstration completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
