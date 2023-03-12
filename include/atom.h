#ifndef ATOM_H
#define ATOM_H

#include <chemfiles.hpp>
#include "graph.h"
#include "bond.h"
#include "element.h"
#include <string>
#include <optional>

namespace MolCpp
{

    class Atom: public Node
    {
        public:
            Atom(const chemfiles::Atom&);
            Atom(const Element&);
            Atom() = default;

            Element get_element() const { return _element; }

        private:

            const Element& _element;
            std::string _name;
            std::string _type;
            double _mass;
            double _charge;
    };

}

#endif  // ATOM_H