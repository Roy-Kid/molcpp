#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "graph.h"
#include "atom.h"
#include "bond.h"

namespace MolCpp
{

    using AtomVec = std::vector<Atom*>;
    using BondVec = std::vector<Bond*>;

    class Topology: public Graph
    {

        public:
            Topology() = default;
            Topology(const chemfiles::Topology& chflTopology);
            void add_atom(Atom* atom) { _atoms.push_back(atom); _nodes.push_back(atom); }
            AtomVec get_atoms() const { return _atoms; }
            void add_bond(Bond* bond) { _bonds.push_back(bond); _edges.push_back(bond); }
            BondVec get_bonds() const { return _bonds; }
            size_t get_natoms() const { return _atoms.size(); }
            size_t get_nbonds() const { return _bonds.size(); }

        protected:
            
            AtomVec _atoms;
            BondVec _bonds;

    };

}

#endif // TOPOLOGY_H