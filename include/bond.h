#ifndef BOND_H
#define BOND_H

#include "graph.h"
#include "atom.h"

namespace MolCpp
{
    class Atom;

    class Bond: public Edge
    {
        public:
            Bond(Atom*, Atom*);

        private:
            

    };

}

#endif  // BOND_H