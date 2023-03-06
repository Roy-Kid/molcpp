#include <gtest/gtest.h>
#include "graph.h"

// Demonstrate some basic assertions.
TEST(TestNode, BasicAssertions) {
    MolCpp::Node node;
    EXPECT_EQ(0, node.GetNumEdges());
}