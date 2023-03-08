#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "graph.h"
#include "atom.h"
#include "bond.h"

namespace MolCpp
{

    class Topology: public Graph
    {

        public:
            Topology() = default;
            Topology(const chemfiles::Topology& chflTopology);
            void add_atom(const Atom& atom) { _atoms.push_back(atom); }
            std::vector<Atom> get_atoms() const { return _atoms; }
            void add_bond(const Bond& bond) { _bonds.push_back(bond); }
            std::vector<Bond> get_bonds() const { return _bonds; }

        private:
            std::vector<Atom> _atoms;
            std::vector<Bond> _bonds;
            // std::vector<Angle> _angles;

    };

}

#endif // TOPOLOGY_H