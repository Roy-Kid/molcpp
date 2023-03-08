#include "bond.h"

namespace MolCpp
{


    Bond::Bond(Atom* begin, Atom* end): 
    Edge {begin, end}
    {}

}