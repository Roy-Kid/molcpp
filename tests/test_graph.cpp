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

    EXPECT_EQ(graph.get_local_index(n1), 0);
    EXPECT_EQ(graph.get_local_index(n2), 1);
    EXPECT_EQ(graph.get_local_index(n3), 2);

    EXPECT_EQ(n1->get_neighbors().size(), 2);
    EXPECT_EQ(n2->get_neighbors().size(), 2);
    EXPECT_EQ(n3->get_neighbors().size(), 2);

    auto t = graph.find_three_bodies();
    EXPECT_EQ(t.size(), 3);
}

TEST(TestEdge, test_delete_Edge)
{

    auto n1 = new MolCpp::Node();
    auto n2 = new MolCpp::Node();
    auto b1 = new MolCpp::Edge(n1, n2);

    EXPECT_EQ(n1->get_nedges(), 1);
    EXPECT_EQ(n2->get_nedges(), 1);

    delete b1;
    EXPECT_EQ(n1->get_nedges(), 0);
    EXPECT_EQ(n2->get_nedges(), 0);

}

TEST(TestGraph, test_subgraph_reduce)
{
    auto subgraph = new MolCpp::Graph();
    auto n1 = new MolCpp::Node();
    auto n2 = new MolCpp::Node();
    auto n3 = new MolCpp::Node();
    subgraph->add_node(n1);
    subgraph->add_node(n2);
    subgraph->add_node(n3);
    auto b1 = new MolCpp::Edge(n1, n2);
    auto b2 = new MolCpp::Edge(n2, n3);
    auto b3 = new MolCpp::Edge(n3, n1);
    subgraph->add_edge(b1);
    subgraph->add_edge(b2);
    subgraph->add_edge(b3);

    auto graph = new MolCpp::Graph();
    graph->add_subgraph(subgraph);

    EXPECT_EQ(graph->get_nnodes(), 3);
    EXPECT_EQ(graph->get_nedges(), 3);

}

TEST(TestGraph, test_subgraph_linkage)
{

    auto subgraph = new MolCpp::Graph();
    auto n1 = new MolCpp::Node();
    auto n2 = new MolCpp::Node();
    auto n3 = new MolCpp::Node();
    subgraph->add_node(n1);
    subgraph->add_node(n2);
    subgraph->add_node(n3);
    auto b1 = new MolCpp::Edge(n1, n2);
    auto b2 = new MolCpp::Edge(n2, n3);
    auto b3 = new MolCpp::Edge(n3, n1);
    subgraph->add_edge(b1);
    subgraph->add_edge(b2);
    subgraph->add_edge(b3);

    auto graph = new MolCpp::Graph();
    graph->add_subgraph(subgraph);

    auto n4 = new MolCpp::Node();
    auto b4 = new MolCpp::Edge(n4, n1);
    graph->add_node(n4);
    graph->add_edge(b4);

    EXPECT_EQ(graph->get_nnodes(), 4);
    EXPECT_EQ(graph->get_nedges(), 4);
    EXPECT_EQ(n1->get_neighbors().size(), 3);
    EXPECT_EQ(graph->find_three_bodies().size(), 5);
}