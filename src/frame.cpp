#include "frame.h"

namespace MolCpp
{
    xt::xarray<double> copy_to_xarray(const std::vector<chemfiles::Vector3D>& vec, std::vector<size_t> shape)
    {
        
        std::vector<double> temp_arr(shape[0]*shape[1]);
        for (size_t i = 0; i < shape[0]; i++)
        {
            for (size_t j = 0; j < shape[1]; j++)
            {
                temp_arr[i*shape[1] + j] = vec[i][j];
            }
        }
        return xt::adapt(temp_arr, shape);
    }

    Frame::Frame(const chemfiles::Frame& chflFrame): _natoms(chflFrame.size()), _current_step(chflFrame.step())
    {

        std::vector<std::size_t> shape = {_natoms, 3};

        _xyz = copy_to_xarray(chflFrame.positions(), {_natoms, 3});

        // copy atom to _atoms
        for (size_t i = 0; i < _natoms; i++)
        {
            _atoms.push_back(Atom(chflFrame[i]));
        }

        

    }

}