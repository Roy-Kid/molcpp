#include <catch2/catch_test_macros.hpp>
#include <molcpp/core/frame.hpp>

using namespace molcpp;

TEST_CASE("Frame basic functionality", "[core][frame]") {
    SECTION("Frame creation") {
        Frame frame;
        
        // Test basic frame properties
        REQUIRE(frame.empty());
        REQUIRE(frame.size() == 0);
    }
    
    SECTION("Frame with atoms") {
        Frame frame;
        
        // Add some test atoms
        // This would depend on the actual Frame implementation
        // REQUIRE(frame.getNumAtoms() == 0);
    }
}
