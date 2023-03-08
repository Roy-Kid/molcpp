#ifndef FRAME_H
#define FRAME_H

#include <chemfiles.hpp>
#include <vector>
#include "xtensor/xtensor.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xadapt.hpp"
#include "atom.h"

namespace MolCpp
{
    class Frame
    {
        public:
            Frame(const chemfiles::Frame&);
            size_t GetNatoms() const { return _natoms; }
            size_t GetCurrentStep() const { return _current_step; }
            xt::xarray<double> GetXYZ() const { return _xyz; }

        private:
            // Topology
            // Cell
            size_t _natoms;
            size_t _current_step;
            std::vector<Atom> _atoms;
            xt::xarray<double> _xyz;

    };

}

#endif // FRAME_H