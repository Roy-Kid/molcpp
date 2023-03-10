#include "element.h"

namespace MolCpp
{

    Element::Element(unsigned int atomicNumber, std::string symbol, double mass, double enegAllred, double covRad, double vdwRad, unsigned int maxbonds, double atomicMass, double eneg_pauling, double ionization, double electronAffinity, double r, double g, double b, std::string elemName) : _atomicNumber{atomicNumber}, _symbol{symbol}, _mass{mass}, _enegAllred{enegAllred}, _covRad{covRad}, _vdwRad{vdwRad}, _maxbonds{maxbonds}, _atomicMass{atomicMass}, _enegPauling{eneg_pauling}, _ionization{ionization}, _electronAffinity{electronAffinity}, _r{r}, _g{g}, _b{b}, _elemName{elemName} {}

    const Element get_element(unsigned int atomicNumber)
    {
        return *_elements.at(atomicNumber);
    }
}