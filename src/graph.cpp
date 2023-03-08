#include "graph.h"

namespace MolCpp
{

    // @brief: the number of combinations in a given container
    // @param: v - the container
    // @param: n - the number of elements in each combination
    // @return: the number of combinations
    std::vector<size_t> combination(std::vector<size_t> &v, size_t n)
    {
        if (v.size() < n) throw std::invalid_argument("v.size() < n");
        if (v.size() == n) {
            std::vector<size_t> result(v);
            return result;
        }
        std::vector<size_t> result;
        size_t *ka = new size_t[n]; // dynamically allocate an array of UINTs
        unsigned int ki = n - 1;    // Point ki to the last elemet of the array
        size_t N = v.size();        // N is the number of elements in the set
        ka[ki] = N - 1;             // Prime the last elemet of the array.

        while (true)
        {
            unsigned int tmp = ka[ki]; // Optimization to prevent reading ka[ki] repeatedly

            while (ki) // Fill to the left with consecutive descending values (blue squares)
                ka[--ki] = --tmp;

            for (size_t i = 0; i < n; ++i)
            {
                result.push_back(v[ka[i]]);
            }

            while (--ka[ki] == ki)
            { // Decrement and check if the resulting value equals the index (bright green squares)
                for (size_t i = 0; i < n; ++i)
                {
                    result.push_back(v[ka[i]]);
                }
                if (++ki == n)
                { // Exit condition (all of the values in the array are flush to the left)
                    delete[] ka;
                    return result;
                }
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
            else
            {
                _nbrs.push_back(edge->get_bgn());
            }
        }
        return _nbrs;
    }

    


    void Graph::add_node(Node* node)
    {
        _nodes.push_back(node);
    }

    void Graph::add_edge(Edge* edge)
    {
        _edges.push_back(edge);
    }

    void Graph::add_subgraph(Graph* subgraph)
    {
        _subgraphs.push_back(subgraph);
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
            if (node->get_nedges() > 2)
            {
                three_body[1] = get_local_index(node);
                
                for(auto nbr : node->get_neighbors())
                {
                    nbr_indices.push_back(get_local_index(node));
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