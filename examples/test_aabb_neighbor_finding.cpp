#include <iostream>
#include <chrono>
#include <xtensor/xarray.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xio.hpp>

#include <molcpp/locality/AABBQuery.hpp>
#include <molcpp/spatial/box.hpp>

using namespace molcpp;
using namespace molcpp::locality;

int main() {
    std::cout << "AABB Neighbor Finding Example\n";
    std::cout << "==============================\n\n";

    // Create a cubic box
    const double L = 10.0;
    Box box({L, L, L});
    std::cout << "Created box with dimensions: " << L << " x " << L << " x " << L << "\n";

    // Generate random points
    const size_t N = 1000;
    xt::xarray<double> points = xt::random::rand<double>({N, 3}) * L - L/2.0;
    std::cout << "Generated " << N << " random points\n\n";

    // Create AABB query structure
    AABBQuery query(box, points);

    // Example 1: Ball query - find all neighbors within radius
    std::cout << "Example 1: Ball Query\n";
    std::cout << "---------------------\n";
    {
        double r_max = 1.0;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, true);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto nlist = query.query(points, args);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Found " << nlist.size() << " neighbor pairs within radius " << r_max << "\n";
        std::cout << "Query time: " << duration.count() << " microseconds\n";
        
        // Calculate average number of neighbors per particle
        auto counts = nlist.getNeighborCounts(N);
        double avg_neighbors = xt::mean(counts)();
        std::cout << "Average neighbors per particle: " << avg_neighbors << "\n\n";
    }

    // Example 2: K-nearest neighbors query
    std::cout << "Example 2: K-Nearest Neighbors Query\n";
    std::cout << "------------------------------------\n";
    {
        unsigned int k = 6;
        QueryArgs args = QueryArgs::nearest(k, -1.0, 1.1, true);
        
        // Query only first 100 points for speed
        auto query_points = xt::view(points, xt::range(0, 100), xt::all());
        
        auto start = std::chrono::high_resolution_clock::now();
        auto nlist = query.query(query_points, args);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Found " << k << " nearest neighbors for 100 query points\n";
        std::cout << "Total neighbor pairs: " << nlist.size() << "\n";
        std::cout << "Query time: " << duration.count() << " microseconds\n\n";
    }

    // Example 3: Range query with both r_min and r_max
    std::cout << "Example 3: Range Query\n";
    std::cout << "----------------------\n";
    {
        double r_min = 0.5;
        double r_max = 1.5;
        QueryArgs args = QueryArgs::ball(r_max, r_min, true);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto nlist = query.query(points, args);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Found " << nlist.size() << " neighbor pairs in range [" 
                  << r_min << ", " << r_max << "]\n";
        std::cout << "Query time: " << duration.count() << " microseconds\n\n";
        
        // Show distance statistics
        auto distances = nlist.getDistances();
        if (distances.size() > 0) {
            double min_dist = xt::amin(distances)();
            double max_dist = xt::amax(distances)();
            double mean_dist = xt::mean(distances)();
            
            std::cout << "Distance statistics:\n";
            std::cout << "  Min: " << min_dist << "\n";
            std::cout << "  Max: " << max_dist << "\n";
            std::cout << "  Mean: " << mean_dist << "\n";
        }
    }

    // Example 4: Demonstrate periodic boundaries
    std::cout << "\nExample 4: Periodic vs Non-Periodic Boundaries\n";
    std::cout << "----------------------------------------------\n";
    {
        // Create a point near the edge
        xt::xarray<double> edge_points = xt::zeros<double>({1, 3});
        edge_points(0, 0) = L/2.0 - 0.1;  // Near the positive x boundary
        
        double r_max = 0.5;
        QueryArgs args = QueryArgs::ball(r_max, 0.0, false);  // Include self
        
        // Query with periodic boundaries
        auto nlist_periodic = query.query(edge_points, args);
        
        // Create effectively non-periodic box (very large)
        Box non_periodic_box({L*1000, L*1000, L*1000});
        AABBQuery query_np(non_periodic_box, points);
        
        auto nlist_non_periodic = query_np.query(edge_points, args);
        
        std::cout << "Point at edge (" << edge_points(0, 0) << ", 0, 0):\n";
        std::cout << "  Neighbors with periodic boundaries: " << nlist_periodic.size() << "\n";
        std::cout << "  Neighbors without periodic boundaries: " << nlist_non_periodic.size() << "\n";
    }

    std::cout << "\nExample complete!\n";

    return 0;
}