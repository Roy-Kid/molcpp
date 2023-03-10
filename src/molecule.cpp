#include "molecule.h"

namespace MolCpp
{

    Molecule::Molecule()
    {
        //ctor
    }

    Molecule::Molecule(const Pattern* p): Topology{}
    {
        for (int i = 0; i < p->acount; i++)
        {
            Atom* a = new Atom();
            this->add_atom(a);
        }
    }

}