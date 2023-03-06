#ifndef ATOM_H
#define ATOM_H

#include "graph.h"
#include "bond.h"

namespace MolCpp
{

    class Atom: public Node
    {
        public:
            Atom() {}

        protected:

            int _element;
            char _type[]; 

        private:

    };

}

#endif  // ATOM_H