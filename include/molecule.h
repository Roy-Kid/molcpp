#ifndef MOLECULE_H
#define MOLECULE_H

#include "topology.h"
#include "parsmart.h"
#include "atom.h"
#include "bond.h"

namespace MolCpp
{

    class Molecule: public Topology
    {

        public:
            Molecule();
            Molecule(const Pattern* );

    };

}

#endif