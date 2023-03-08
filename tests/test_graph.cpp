#include "utils.h"
#include "graph.h"

TEST(TestGraph, test_init)
{

    MolCpp::Graph graph;
    auto n1 = new MolCpp::Node();
    auto n2 = new MolCpp::Node();
    auto n3 = new MolCpp::Node();
    graph.add_node(n1);
    graph.add_node(n2);
    graph.add_node(n3);
    auto b1 = new MolCpp::Edge(n1, n2);
    auto b2 = new MolCpp::Edge(n2, n3);
    auto b3 = new MolCpp::Edge(n3, n1);
    graph.add_edge(b1);
    graph.add_edge(b2);
    graph.add_edge(b3);

    EXPECT_EQ(graph.get_nnodes(), 3);
    EXPECT_EQ(graph.get_nedges(), 3);

}