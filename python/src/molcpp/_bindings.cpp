// _bindings.cpp
// Python bindings for molcpp using xtensor-python

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#define FORCE_IMPORT_ARRAY
#include <xtensor-python/pyarray.hpp>
#include <xtensor-python/pytensor.hpp>
#include <xtensor-python/pyvectorize.hpp>

#include <molcpp/types.hpp>
#include <molcpp/atom.hpp>
#include <molcpp/spatial/region.hpp>
#include <molcpp/spatial/box.hpp>

namespace py = pybind11;

// Utility functions for xtensor bindings
namespace molcpp_python {

/**
 * @brief Convert Vec3 to Python array
 */
xt::pyarray<double> vec3_to_pyarray(const Vec3& vec) {
    return xt::pyarray<double>(vec);
}

/**
 * @brief Convert Python array to Vec3
 */
Vec3 pyarray_to_vec3(const xt::pyarray<double>& arr) {
    if (arr.size() != 3) {
        throw std::invalid_argument("Array must have exactly 3 elements");
    }
    Vec3 result;
    std::copy(arr.begin(), arr.end(), result.begin());
    return result;
}

/**
 * @brief Convert Mat3 to Python array
 */
xt::pyarray<double> mat3_to_pyarray(const Mat3& mat) {
    return xt::pyarray<double>(mat);
}

/**
 * @brief Convert Python array to Mat3
 */
Mat3 pyarray_to_mat3(const xt::pyarray<double>& arr) {
    if (arr.shape().size() != 2 || arr.shape(0) != 3 || arr.shape(1) != 3) {
        throw std::invalid_argument("Array must be 3x3");
    }
    Mat3 result;
    std::copy(arr.begin(), arr.end(), result.begin());
    return result;
}

} // namespace molcpp_python

PYBIND11_MODULE(_bindings, m) {
    xt::import_numpy();
    
    m.doc() = "molcpp: Molecular modeling library with xtensor integration";
    
    // Types module
    py::module_ types = m.def_submodule("types", "Basic types and utilities");
    
    // Vec3 bindings
    py::class_<Vec3>(types, "Vec3")
        .def(py::init<>())
        .def(py::init([](const xt::pyarray<double>& arr) {
            return molcpp_python::pyarray_to_vec3(arr);
        }))
        .def("to_numpy", &molcpp_python::vec3_to_pyarray)
        .def("__getitem__", [](const Vec3& v, size_t i) { return v[i]; })
        .def("__setitem__", [](Vec3& v, size_t i, double val) { v[i] = val; })
        .def("__len__", [](const Vec3&) { return 3; })
        .def("__repr__", [](const Vec3& v) {
            return "Vec3([" + std::to_string(v[0]) + ", " + 
                   std::to_string(v[1]) + ", " + std::to_string(v[2]) + "])";
        });
    
    // Mat3 bindings
    py::class_<Mat3>(types, "Mat3")
        .def(py::init<>())
        .def(py::init([](const xt::pyarray<double>& arr) {
            return molcpp_python::pyarray_to_mat3(arr);
        }))
        .def("to_numpy", &molcpp_python::mat3_to_pyarray)
        .def("__getitem__", [](const Mat3& m, std::pair<size_t, size_t> idx) { 
            return m(idx.first, idx.second); 
        })
        .def("__setitem__", [](Mat3& m, std::pair<size_t, size_t> idx, double val) { 
            m(idx.first, idx.second) = val; 
        })
        .def("shape", [](const Mat3&) { return std::make_pair(3, 3); })
        .def("__repr__", [](const Mat3& m) {
            std::string result = "Mat3([[";
            for (size_t i = 0; i < 3; ++i) {
                if (i > 0) result += "], [";
                for (size_t j = 0; j < 3; ++j) {
                    if (j > 0) result += ", ";
                    result += std::to_string(m(i, j));
                }
            }
            result += "]])";
            return result;
        });
    
    // Atom bindings
    py::module_ atom_module = m.def_submodule("atom", "Atom-related classes");
    
    py::class_<molcpp::Atom>(atom_module, "Atom")
        .def(py::init<>())
        .def("__repr__", [](const molcpp::Atom&) {
            return "<molcpp.Atom>";
        });
    
    // Spatial module
    py::module_ spatial = m.def_submodule("spatial", "Spatial analysis tools");
    
    // Region base class
    py::class_<molcpp::Region>(spatial, "Region")
        .def("isin", [](const molcpp::Region& self, const xt::pyarray<double>& coords) {
            return self.isin(coords);
        }, "Check which particles are inside this region")
        .def("boundary", &molcpp::Region::boundary, 
             "Get bounding box as [xlo, xhi, ylo, yhi, zlo, zhi]")
        .def("volume", &molcpp::Region::volume, "Get volume of the region");
    
    // Utility functions for array operations
    m.def("array_example", [](const xt::pyarray<double>& input) -> xt::pyarray<double> {
        // Example function showing xtensor operations
        return input * 2.0 + 1.0;
    }, "Example function demonstrating xtensor array operations");
    
    m.def("vec3_operations", [](const xt::pyarray<double>& coords) -> xt::pyarray<double> {
        // Example: compute norms of 3D vectors
        // coords should be shape (n, 3)
        if (coords.dimension() != 2 || coords.shape(1) != 3) {
            throw std::invalid_argument("coords must have shape (n, 3)");
        }
        
        // Compute squared norms using xtensor
        auto norms_squared = xt::sum(coords * coords, {1});
        return xt::sqrt(norms_squared);
    }, "Compute norms of 3D vectors");
    
    m.def("distance_matrix", [](const xt::pyarray<double>& coords1, 
                                const xt::pyarray<double>& coords2) -> xt::pyarray<double> {
        // Compute pairwise distances between two sets of points
        if (coords1.dimension() != 2 || coords1.shape(1) != 3 ||
            coords2.dimension() != 2 || coords2.shape(1) != 3) {
            throw std::invalid_argument("Both coordinate arrays must have shape (n, 3)");
        }
        
        size_t n1 = coords1.shape(0);
        size_t n2 = coords2.shape(0);
        
        xt::xarray<double> result = xt::empty<double>({n1, n2});
        
        // Compute pairwise distances
        for (size_t i = 0; i < n1; ++i) {
            for (size_t j = 0; j < n2; ++j) {
                auto diff = xt::view(coords1, i, xt::all()) - xt::view(coords2, j, xt::all());
                result(i, j) = xt::sqrt(xt::sum(diff * diff))();
            }
        }
        
        return xt::pyarray<double>(result);
    }, "Compute pairwise distance matrix between two coordinate sets");
}