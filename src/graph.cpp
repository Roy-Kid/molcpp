#include "graph.h"

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

    void Node::del_edge(Edge* edge)
    {
        for (auto e : _edges)
        {
            if (e == edge)
            {
                _edges.erase(std::remove(_edges.begin(), _edges.end(), edge), _edges.end());
            }
        }
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

    Edge::~Edge()
    {
        if (_bgn != nullptr)
        {
            _bgn->del_edge(this);
        }
        if (_end != nullptr)
        {
            _end->del_edge(this);
        }
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

    NodeVec Graph::get_nodes()
    {
        NodeVec nodes = _nodes;
        for (auto subgraph : _subgraphs)
        {
            NodeVec subnodes = subgraph->get_nodes();
            nodes.insert(nodes.end(), subnodes.begin(), subnodes.end());
        }
        return nodes;
    }

    EdgeVec Graph::get_edges()
    {
        EdgeVec edges = _edges;
        for (auto subgraph : _subgraphs)
        {
            EdgeVec subedges = subgraph->get_edges();
            edges.insert(edges.end(), subedges.begin(), subedges.end());
        }
        return edges;
    }

    size_t Graph::get_nnodes()
    {

        size_t nnodes = _nodes.size();
        for (auto subgraph : _subgraphs)
        {
            nnodes += subgraph->get_nnodes();
        }
        return nnodes;

    }

    size_t Graph::get_nedges()
    {
        size_t nedges = _edges.size();
        for (auto subgraph : _subgraphs)
        {
            nedges += subgraph->get_nedges();
        }
        return nedges;
    }

    size_t Graph::get_local_index(Node* node)
    {
        auto ans = find_in_container(get_nodes(), node);
        if (ans.has_value())
        {
            return ans.value();
        }
        else
        {
            throw std::runtime_error("Node not found in graph");
        }
    }

    ThreeBodyIndex Graph::find_three_bodies()
    {
        ThreeBodyIndex three_bodies;
        std::vector<size_t> nbr_indices;
        std::array<size_t, 3> three_body;

        auto nodes = get_nodes();

        for (auto node : get_nodes())
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
            nbr_indices.clear();
            
        }

        return three_bodies;
    }


}