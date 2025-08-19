#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/locality/AABB.hpp"
#include "molcpp/locality/AABB_simd.hpp"
#include <vector>
#include <random>

using namespace molcpp;
using namespace molcpp::locality;

TEST_CASE("SIMD AABB operations", "[locality][aabb][simd]")
{
    SECTION("Debug SIMD implementation")
    {
        // Create simple test AABBs
        AABB a(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{2.0f, 2.0f, 2.0f});
        AABB b(Vec3<float>{1.0f, 1.0f, 1.0f}, Vec3<float>{3.0f, 3.0f, 3.0f});
        
        // Test regular overlap
        bool regular_result = overlap(a, b);
        std::cout << "Regular overlap result: " << regular_result << std::endl;
        
        // Test SIMD overlap
        bool simd_result = simd::overlap(a, b);
        std::cout << "SIMD overlap result: " << simd_result << std::endl;
        
        // Debug: print AABB data
        std::cout << "AABB a - lower: [" << a.lower[0] << ", " << a.lower[1] << ", " << a.lower[2] << "]" << std::endl;
        std::cout << "AABB a - upper: [" << a.upper[0] << ", " << a.upper[1] << ", " << a.upper[2] << "]" << std::endl;
        std::cout << "AABB b - lower: [" << b.lower[0] << ", " << b.lower[1] << ", " << b.lower[2] << "]" << std::endl;
        std::cout << "AABB b - upper: [" << b.upper[0] << ", " << b.upper[1] << ", " << b.upper[2] << "]" << std::endl;
        
        // Check if they should overlap
        bool should_overlap = (b.upper[0] >= a.lower[0] && b.lower[0] <= a.upper[0] &&
                              b.upper[1] >= a.lower[1] && b.lower[1] <= a.upper[1] &&
                              b.upper[2] >= a.lower[2] && b.lower[2] <= a.upper[2]);
        std::cout << "Should overlap: " << should_overlap << std::endl;
        
        CHECK(regular_result == should_overlap);
        CHECK(simd_result == should_overlap);
    }
    
    SECTION("SIMD overlap test")
    {
        // Create test AABBs
        AABB a(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{2.0f, 2.0f, 2.0f});
        AABB b(Vec3<float>{1.0f, 1.0f, 1.0f}, Vec3<float>{3.0f, 3.0f, 3.0f});
        AABB c(Vec3<float>{4.0f, 4.0f, 4.0f}, Vec3<float>{6.0f, 6.0f, 6.0f});
        
        // Test SIMD overlap
        CHECK(simd::overlap(a, b) == true);   // Should overlap
        CHECK(simd::overlap(a, c) == false);  // Should not overlap
        
        // Compare with regular overlap function
        CHECK(simd::overlap(a, b) == overlap(a, b));
        CHECK(simd::overlap(a, c) == overlap(a, c));
    }
    
    SECTION("SIMD AABB-Sphere overlap test")
    {
        AABB aabb(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{2.0f, 2.0f, 2.0f});
        AABBSphere sphere1(Vec3<float>{1.0f, 1.0f, 1.0f}, 0.5f);  // Inside AABB
        AABBSphere sphere2(Vec3<float>{3.0f, 3.0f, 3.0f}, 0.5f);  // Outside AABB
        
        // Test SIMD overlap
        CHECK(simd::overlap(aabb, sphere1) == true);   // Should overlap
        CHECK(simd::overlap(aabb, sphere2) == false);  // Should not overlap
        
        // Compare with regular overlap function
        CHECK(simd::overlap(aabb, sphere1) == overlap(aabb, sphere1));
        CHECK(simd::overlap(aabb, sphere2) == overlap(aabb, sphere2));
    }
    
    SECTION("SIMD contains test")
    {
        AABB outer(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{4.0f, 4.0f, 4.0f});
        AABB inner(Vec3<float>{1.0f, 1.0f, 1.0f}, Vec3<float>{3.0f, 3.0f, 3.0f});
        AABB partial(Vec3<float>{1.0f, 1.0f, 1.0f}, Vec3<float>{5.0f, 3.0f, 3.0f});
        
        // Test SIMD contains
        CHECK(simd::contains(outer, inner) == true);    // inner is fully contained
        CHECK(simd::contains(outer, partial) == false); // partial extends beyond
        
        // Compare with regular contains function
        CHECK(simd::contains(outer, inner) == contains(outer, inner));
        CHECK(simd::contains(outer, partial) == contains(outer, partial));
    }
    
    SECTION("SIMD merge test")
    {
        AABB a(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{2.0f, 2.0f, 2.0f});
        AABB b(Vec3<float>{1.0f, 1.0f, 1.0f}, Vec3<float>{3.0f, 3.0f, 3.0f});
        
        // Test SIMD merge
        AABB merged_simd = simd::merge(a, b);
        AABB merged_regular = merge(a, b);
        
        // Results should be identical
        CHECK(merged_simd.lower[0] == Catch::Approx(merged_regular.lower[0]));
        CHECK(merged_simd.lower[1] == Catch::Approx(merged_regular.lower[1]));
        CHECK(merged_simd.lower[2] == Catch::Approx(merged_regular.lower[2]));
        CHECK(merged_simd.upper[0] == Catch::Approx(merged_regular.upper[0]));
        CHECK(merged_simd.upper[1] == Catch::Approx(merged_regular.upper[1]));
        CHECK(merged_simd.upper[2] == Catch::Approx(merged_regular.upper[2]));
    }
    
    SECTION("Batch overlap test")
    {
        // Create a large number of AABBs
        const size_t num_aabbs = 1000;
        std::vector<AABB> aabbs;
        bool results_simd[1000];  // Use regular array instead of vector<bool>
        bool results_regular[1000];
        
        // Generate random AABBs
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> pos_dist(-10.0f, 10.0f);
        std::uniform_real_distribution<float> size_dist(0.1f, 2.0f);
        
        for (size_t i = 0; i < num_aabbs; ++i)
        {
            Vec3<float> pos{pos_dist(gen), pos_dist(gen), pos_dist(gen)};
            float size = size_dist(gen);
            aabbs.emplace_back(pos, size);
        }
        
        // Query AABB
        AABB query_aabb(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{1.0f, 1.0f, 1.0f});
        
        // Test batch overlap
        simd::batch_overlap(&aabbs[0], query_aabb, results_simd, num_aabbs);
        
        // Compare with regular overlap
        for (size_t i = 0; i < num_aabbs; ++i)
        {
            results_regular[i] = overlap(aabbs[i], query_aabb);
            CHECK(results_simd[i] == results_regular[i]);
        }
    }
    
    SECTION("Performance comparison")
    {
        const size_t num_tests = 10000;
        std::vector<AABB> aabbs;
        
        // Generate test data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> pos_dist(-5.0f, 5.0f);
        std::uniform_real_distribution<float> size_dist(0.1f, 1.0f);
        
        for (size_t i = 0; i < num_tests; ++i)
        {
            Vec3<float> pos{pos_dist(gen), pos_dist(gen), pos_dist(gen)};
            float size = size_dist(gen);
            aabbs.emplace_back(pos, size);
        }
        
        AABB query_aabb(Vec3<float>{0.0f, 0.0f, 0.0f}, Vec3<float>{1.0f, 1.0f, 1.0f});
        
        // Test regular overlap performance
        volatile int regular_result = 0;  // volatile to prevent optimization
        auto start_regular = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < num_tests; ++i)
        {
            regular_result += overlap(aabbs[i], query_aabb) ? 1 : 0;
        }
        auto end_regular = std::chrono::high_resolution_clock::now();
        auto duration_regular = std::chrono::duration_cast<std::chrono::microseconds>(end_regular - start_regular);
        
        // Test SIMD overlap performance
        volatile int simd_result = 0;  // volatile to prevent optimization
        auto start_simd = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < num_tests; ++i)
        {
            simd_result += simd::overlap(aabbs[i], query_aabb) ? 1 : 0;
        }
        auto end_simd = std::chrono::high_resolution_clock::now();
        auto duration_simd = std::chrono::duration_cast<std::chrono::microseconds>(end_simd - start_simd);
        
        // Output performance comparison
        std::cout << "Regular overlap: " << duration_regular.count() << " microseconds (result: " << regular_result << ")" << std::endl;
        std::cout << "SIMD overlap: " << duration_simd.count() << " microseconds (result: " << simd_result << ")" << std::endl;
        std::cout << "Speedup: " << static_cast<float>(duration_regular.count()) / duration_simd.count() << "x" << std::endl;
        
        // Verify that both functions produce the same results
        CHECK(regular_result == simd_result);
        
        // Now we can properly compare performance
        if (duration_regular.count() > 0 && duration_simd.count() > 0)
        {
            std::cout << "Regular: " << duration_regular.count() << " μs, SIMD: " << duration_simd.count() << " μs" << std::endl;
            // SIMD should ideally be faster, but at least not much slower
            CHECK(duration_simd.count() <= duration_regular.count() * 2);  // Allow 2x slower for now
        }
    }
}
