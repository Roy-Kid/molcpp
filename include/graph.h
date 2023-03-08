#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <array>
#include <algo.h>

namespace MolCpp
{

    class Graph;
    class Edge;
    class Node;
    using NodeVec = std::vector<Node*>;
    using EdgeVec = std::vector<Edge*>;
    using ThreeBodyIndex = std::vector<std::array<size_t, 3>>;

    class Node
    {

        public:
            Node(): _parent{nullptr} {}
            int get_nedges() const { return _edges.size(); }
            void set_parent(Graph* parent) { _parent = parent; }
            EdgeVec get_edges() const { return _edges; }
            void add_edge(Edge*);
            bool has_edge(Edge*);
            NodeVec get_neighbors();

        protected:
            Graph* _parent;
            EdgeVec _edges;

    };


    class Edge
    {
        public:
            Edge(): _parent{nullptr}, _bgn{nullptr}, _end{nullptr} {}
            Edge(Node* begin, Node* end): _parent{nullptr} {
                begin->add_edge(this);
                end->add_edge(this);
            }
            Node* get_bgn() const { return _bgn; }
            Node* get_end() const { return _end; }
            void set_parent(Graph* parent) { _parent = parent; }

        protected:
            Graph* _parent;
            Node* _bgn;
            Node* _end;

    };


    class Graph
    {
        public:
            Graph() = default;
            void add_node(Node*);
            void add_edge(Edge*);
            void add_subgraph(Graph*);
            void del_node(Node*);
            void del_edge(Edge*);
            size_t get_nnodes() const { return _nodes.size(); }
            size_t get_nedges() const { return _edges.size(); }
            size_t get_local_index(Node* node);
            void set_parent(Graph* parent) { _parent = parent; }

            ThreeBodyIndex find_three_bodies();

        protected:
            Graph* _parent;
            std::vector<Node*> _nodes;
            std::vector<Edge*> _edges;
            std::vector<Graph*> _subgraphs;

    };

}

#endif  // GRAPH_H