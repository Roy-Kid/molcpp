#include "topology.h"

namespace MolCpp
{

    Topology::Topology(const chemfiles::Topology& chflTopology) : Graph {}
    {
        // copy atoms
        for (size_t i = 0; i < chflTopology.size(); i++)
        {
            auto _atom = Atom(chflTopology[i]);
            _atom.set_parent(this);
            _atoms.push_back(_atom);
        }

        // copy bonds
        for (size_t i = 0; i < chflTopology.bonds().size(); i++)
        {

            auto _chflBond = chflTopology.bonds()[i];
            auto _bond =  Bond(&_atoms[_chflBond[0]], &_atoms[_chflBond[1]]);
            _bond.set_parent(this);
            _bonds.push_back(_bond);
        }




    }

}