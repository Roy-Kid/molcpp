#include "atom.h"

namespace MolCpp
{

    Atom::Atom(const chemfiles::Atom& chflAtom):
    Node {},
    _name(chflAtom.name()),
    _type(chflAtom.type()),
    _mass(chflAtom.mass()),
    _charge(chflAtom.charge()),
    _atomic_number(chflAtom.atomic_number())
    {
        
    }

}