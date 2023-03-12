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

        private:

            const Element& _element;
            std::string _name;
            std::string _type;
            double _mass;
            double _charge;
            std::experimental::optional<uint64_t> _atomic_number;
    };

}

#endif  // ATOM_H