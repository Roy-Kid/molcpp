#include "graph.h"
#include <iostream>
namespace MolCpp
{

    void Node::add_edge(Edge* edge)
    {
        if (!has_edge(edge))
        {
            _edges.push_back(edge);
        }

    }

    bool Node::has_edge(Edge* edge)
    {
        for (auto e : _edges)
        {
            if (e == edge)
            {
                return true;
            }
        }
        return false;
    }


    NodeVec Node::get_neighbors()
    {
        NodeVec _nbrs;
        for (auto edge : _edges)
        {
            if (edge->get_bgn() == this)
            {
                _nbrs.push_back(edge->get_end());
            }
            else if (edge->get_end() == this)
            {
                _nbrs.push_back(edge->get_bgn());
            }
        }
        return _nbrs;
    }

    


    void Graph::add_node(Node* node)
    {
        _nodes.push_back(node);
        node->set_parent(this);
    }

    void Graph::add_edge(Edge* edge)
    {
        _edges.push_back(edge);
        edge->set_parent(this);
    }

    void Graph::add_subgraph(Graph* subgraph)
    {
        _subgraphs.push_back(subgraph);
        subgraph->set_parent(this);
    }

    void Graph::del_node(Node* node)
    {
        for (auto n = _nodes.begin(); n != _nodes.end(); n++)
        {
            if (*n == node)
            {
                _nodes.erase(n);
                break;
            }
        }
        for (auto e = _edges.begin(); e != _edges.end(); e++)
        {
            if ((*e)->get_bgn() == node || (*e)->get_end() == node)
            {
                _edges.erase(e);
            }
        }
    }

    size_t Graph::get_local_index(Node* node)
    {
        for (auto i = 0; i < _nodes.size(); i++)
        {
            if (_nodes[i] == node)
            {
                return i;
            }
        }
        throw std::invalid_argument("node not found in graph");
        
    }

    ThreeBodyIndex Graph::find_three_bodies()
    {
        std::vector<size_t> nbr_indices;
        ThreeBodyIndex three_bodies;
        std::array<size_t, 3> three_body;
        for (auto node : _nodes)
        {
            if (node->get_nedges() > 1)
            {
                three_body[1] = get_local_index(node);
                
                for(auto nbr : node->get_neighbors())
                {
                    nbr_indices.push_back(get_local_index(nbr));
                }

                auto combinations = combination(nbr_indices, 2);
                for (auto i = 0; i < combinations.size(); i += 2)
                {
                    three_body[0] = combinations[i];
                    three_body[2] = combinations[i+1];
                    three_bodies.push_back(three_body);
                }

            }
            
        }

        return three_bodies;
    }


}