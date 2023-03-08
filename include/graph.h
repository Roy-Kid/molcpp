#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

namespace MolCpp
{

    class Graph;
    class Edge;

    class Node
    {
        public:
            Node(): _parent{nullptr} {}
            int GetNumEdges() const { return _edges.size(); }
            void set_parent(Graph* parent) { _parent = parent; }

        protected:
            Graph* _parent;
            std::vector<Edge*> _edges;

    };


    class Edge
    {
        public:
            Edge(): _parent{nullptr}, _bgn{nullptr}, _end{nullptr} {}
            Edge(Node* begin, Node* end): _parent{nullptr} {}
            Node* GetBgn() const { return _bgn; }
            Node* GetEnd() const { return _end; }
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
            void AddNode(Node*);
            void AddEdge(Edge*);
            // void RemoveNode(Node*);
            // void RemoveEdge(Edge*);

        protected:
            Graph* _parent;
            std::vector<Node*> _nodes;
            std::vector<Edge*> _edges;

    };

}

#endif  // GRAPH_H