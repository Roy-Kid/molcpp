#include <catch2/catch_test_macros.hpp>
#include "molcpp/locality/AABBQuery.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/types.hpp"

using namespace molcpp;
using namespace molcpp::locality;

TEST_CASE("AABBQuery single point test", "[locality][aabb_single]")
{
    SECTION("Test AABBQuery with single point")
    {
        Vec3<bool> pbc{false, false, false};
        Vec3<float> origin{0.0f, 0.0f, 0.0f};
        Box box = Box::cube(10.0f, origin, pbc);
        
        XYZ single_point = xt::zeros<float>({1, 3});
        single_point(0, 0) = 5.0f;
        single_point(0, 1) = 5.0f;
        single_point(0, 2) = 5.0f;
        
        AABBQuery query(box, single_point);
        // CHECK(query.getNPoints() == 1);
    }
}

// TEST_CASE("AABBQuery two points test", "[locality][aabb_two]")
// {
//     SECTION("Test AABBQuery with two points")
//     {
//         Vec3<bool> pbc{false, false, false};
//         Vec3<float> origin{0.0f, 0.0f, 0.0f};
//         Box box = Box::cube(10.0f, origin, pbc);
        
//         XYZ two_points = xt::zeros<float>({2, 3});
//         two_points(0, 0) = 1.0f; two_points(0, 1) = 1.0f; two_points(0, 2) = 1.0f;
//         two_points(1, 0) = 2.0f; two_points(1, 1) = 2.0f; two_points(1, 2) = 2.0f;
        
//         AABBQuery query(box, two_points);
//         CHECK(query.getNPoints() == 2);
//     }
// }
