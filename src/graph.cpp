#include "graph.h"

namespace MolCpp
{

    void Graph::AddNode(Node* node)
    {
        _nodes.push_back(node);
    }

    void Graph::AddEdge(Edge* edge)
    {
        _edges.push_back(edge);
    }


}