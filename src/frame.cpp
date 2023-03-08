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

    Frame::Frame(const chemfiles::Frame& chflFrame): _current_step(chflFrame.step())
    {

        _xyz = copy_to_xarray(chflFrame.positions(), {get_natoms(), 3});

        // copy topology
        _topology = Topology(chflFrame.topology());


    }

}