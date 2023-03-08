#ifndef ALGO_H
#define ALGO_H

#include <vector>
#include <algorithm>
#include <exception>

namespace MolCpp
{

    // @brief: the number of combinations in a given container
    // @param: v - the container
    // @param: n - the number of elements in each combination
    // @return: the number of combinations
    std::vector<size_t> combination(std::vector<size_t> &, size_t);

}

#endif // ALGO_H