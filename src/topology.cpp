#include "topology.h"

namespace MolCpp
{

    Topology::Topology(const chemfiles::Topology& chflTopology) : Graph {}
    {
        // copy atoms
        for (size_t i = 0; i < chflTopology.size(); i++)
        {
            auto _atom = new Atom(chflTopology[i]);
            _atom->set_parent(this);
            add_atom(_atom);
        }

        // copy bonds
        for (size_t i = 0; i < chflTopology.bonds().size(); i++)
        {

            auto _chflBond = chflTopology.bonds()[i];
            auto _bond = new Bond(_atoms[_chflBond[0]], _atoms[_chflBond[1]]);
            _bond->set_parent(this);
            add_bond(_bond);
        }

        // register subgraph


    }

}