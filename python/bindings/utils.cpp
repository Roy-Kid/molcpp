// utils.cpp
// Python utility bindings for molcpp

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pyarray.hpp>

#include <molcpp/types.hpp>

namespace py = pybind11;

// Helper function to convert numpy array to Vec3
Vec3 pyarray_to_vec3(const xt::pyarray<double>& arr) {
    if (arr.size() != 3) {
        throw std::invalid_argument("Array must have exactly 3 elements for Vec3 conversion");
    }
    Vec3 result;
    for (size_t i = 0; i < 3; ++i) {
        result(i) = arr.flat(i);
    }
    return result;
}

// Helper function to convert Vec3 to numpy array
xt::pyarray<double> vec3_to_pyarray(const Vec3& vec) {
    return xt::pyarray<double>(vec);
}

// Helper function to convert numpy array to Mat3
Mat3 pyarray_to_mat3(const xt::pyarray<double>& arr) {
    if (arr.shape().size() != 2 || arr.shape(0) != 3 || arr.shape(1) != 3) {
        throw std::invalid_argument("Array must be 3x3 for Mat3 conversion");
    }
    Mat3 result;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            result(i, j) = arr(i, j);
        }
    }
    return result;
}

// Helper function to convert Mat3 to numpy array
xt::pyarray<double> mat3_to_pyarray(const Mat3& mat) {
    return xt::pyarray<double>(mat);
}

void bind_utils(py::module_& m) {
    // Utility functions module
    py::module_ utils = m.def_submodule("utils", "Utility functions for type conversions");
    
    // Add conversion functions as module functions
    utils.def("pyarray_to_vec3", &pyarray_to_vec3, 
              "Convert numpy array to Vec3",
              py::arg("arr"));
              
    utils.def("vec3_to_pyarray", &vec3_to_pyarray,
              "Convert Vec3 to numpy array", 
              py::arg("vec"));
              
    utils.def("pyarray_to_mat3", &pyarray_to_mat3,
              "Convert numpy array to Mat3",
              py::arg("arr"));
              
    utils.def("mat3_to_pyarray", &mat3_to_pyarray,
              "Convert Mat3 to numpy array",
              py::arg("mat"));
}
