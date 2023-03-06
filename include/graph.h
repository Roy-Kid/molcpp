#include <vector>

namespace MolCpp
{

    class Node
    {
        public:


        private:
            Graph* _parent;
            std::vector<Edge*> _edges;

    };

    class Edge
    {
        public:

        private:
            Graph* _parent;
            Node _bgn, _end;

    };

    class Graph
    {
        public:
            Graph() = default;
            void AddNode(Node*);
            void AddEdge(Edge*);
            // void RemoveNode(Node*);
            // void RemoveEdge(Edge*);

        private:
            Graph* _parent;
            std::vector<Node*> _nodes;
            std::vector<Edge*> _edges;

    };

}