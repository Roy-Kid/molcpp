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

    Atom::Atom(const Element& element): Node{}, _element{element}, _name{element.get_name()}, _type{element.get_symbol()}, _mass{element.get_mass()}, _charge{0.0}, _atomic_number{element.get_atomic_number()}
    {
        
    }
}