// spatial.cpp
// Python bindings for molcpp spatial classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <xtensor-python/pyarray.hpp>

#include <molcpp/spatial/box.hpp>
#include <molcpp/spatial/boundary.hpp>
#include <molcpp/spatial/region.hpp>
#include <molcpp/types.hpp>

namespace py = pybind11;

// Helper function to convert numpy array to Vec3
Vec3 pyarray_to_vec3(const xt::pyarray<double>& arr) {
    if (arr.size() != 3) {
        throw std::invalid_argument("Array must have exactly 3 elements");
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

// Helper function to convert Mat3 to numpy array
xt::pyarray<double> mat3_to_pyarray(const Mat3& mat) {
    return xt::pyarray<double>(mat);
}

// Helper function to convert numpy array to Mat3
Mat3 pyarray_to_mat3(const xt::pyarray<double>& arr) {
    if (arr.shape().size() != 2 || arr.shape(0) != 3 || arr.shape(1) != 3) {
        throw std::invalid_argument("Array must be 3x3 matrix");
    }
    Mat3 result;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            result(i, j) = arr(i, j);
        }
    }
    return result;
}

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <xtensor-python/pyarray.hpp>
#include <xtensor-python/pytensor.hpp>
#include "molcpp/types.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/spatial/boundary.hpp"
#include "molcpp/spatial/region.hpp"

namespace py = pybind11;

void bind_spatial(py::module& m) {
    auto spatial = m.def_submodule("spatial", "Spatial analysis and region tools");

    // Box::Style enum
    py::enum_<molcpp::Box::Style>(spatial, "BoxStyle")
        .value("FREE", molcpp::Box::Style::FREE)
        .value("ORTHOGONAL", molcpp::Box::Style::ORTHOGONAL)
        .value("TRICLINIC", molcpp::Box::Style::TRICLINIC);

    // Base Region class
    py::class_<molcpp::Region>(spatial, "Region")
        .def("isin", [](const molcpp::Region& self, const xt::pyarray<double>& coords) {
            return xt::pyarray<bool>(self.isin(coords));
        }, "Check which particles are inside this region")
        .def("boundary", &molcpp::Region::boundary, "Get the bounding box of this region")
        .def("volume", &molcpp::Region::volume, "Get the volume of this region");

    // Base Boundary class
    py::class_<molcpp::Boundary>(spatial, "Boundary")
        .def("wrap", [](const molcpp::Boundary& self, const xt::pyarray<double>& coords) {
            return xt::pyarray<double>(self.wrap(coords));
        }, "Wrap coordinates according to boundary conditions")
        .def("minimum_image", [](const molcpp::Boundary& self, 
                                const xt::pyarray<double>& r1, 
                                const xt::pyarray<double>& r2) {
            return xt::pyarray<double>(self.minimum_image(r1, r2));
        }, "Get minimum image distance vector")
        .def("get_bounds", &molcpp::Boundary::get_bounds, "Get bounding box")
        .def("is_periodic", &molcpp::Boundary::is_periodic, "Check periodicity");

    // Box class (inherits from both Region and Boundary)
    py::class_<molcpp::Box, molcpp::Region, molcpp::Boundary>(spatial, "Box")
        .def(py::init<>(), "Initialize free (infinite) box")
        .def(py::init<const Mat3&>(), "Initialize from lattice matrix")
        .def(py::init<const Vec3&>(), "Initialize from lengths vector")
        .def(py::init<const std::initializer_list<double>&>(), "Initialize from lengths list")
        .def(py::init([](const py::list& lengths) {
            if (lengths.size() != 3) {
                throw std::invalid_argument("Lengths list must have exactly 3 elements");
            }
            Vec3 vec;
            vec(0) = lengths[0].cast<double>();
            vec(1) = lengths[1].cast<double>();
            vec(2) = lengths[2].cast<double>();
            return molcpp::Box(vec);
        }), "Initialize from Python list of lengths")
        
        // Basic properties
        .def("get_style", &molcpp::Box::get_style, "Get the box style")
        .def("get_lengths", &molcpp::Box::get_lengths, "Get the box lengths")
        .def("get_angles", &molcpp::Box::get_angles, "Get the box angles")
        .def("get_volume", &molcpp::Box::get_volume, "Get the box volume")
        
        // Matrix operations
        .def("get_matrix", [](const molcpp::Box& self) {
            return xt::pyarray<double>(self.get_matrix());
        }, "Get the lattice matrix")
        .def("get_inv", [](const molcpp::Box& self) {
            return xt::pyarray<double>(self.get_inv());
        }, "Get the inverse lattice matrix")
        
        // Setters
        .def("set_lengths", &molcpp::Box::set_lengths, "Set box lengths")
        .def("set_angles", &molcpp::Box::set_angles, "Set box angles")
        .def("set_matrix", &molcpp::Box::set_matrix, "Set lattice matrix")
        .def("set_lengths_angles", &molcpp::Box::set_lengths_angles, 
             "Set box lengths and angles", py::arg("lengths"), py::arg("angles"))
        
        // Coordinate operations  
        .def("wrap", [](const molcpp::Box& self, const xt::pyarray<double>& coords) {
            return xt::pyarray<double>(self.wrap(coords));
        }, "Wrap coordinates to box")
        .def("minimum_image", [](const molcpp::Box& self, 
                                const xt::pyarray<double>& r1, 
                                const xt::pyarray<double>& r2) {
            return xt::pyarray<double>(self.minimum_image(r1, r2));
        }, "Get minimum image distance vector")
        
        // Boundary interface
        .def("get_bounds", &molcpp::Box::get_bounds, "Get bounding box")
        .def("is_periodic", &molcpp::Box::is_periodic, "Check periodicity")
        
        // Region interface
        .def("isin", [](const molcpp::Box& self, const xt::pyarray<double>& coords) {
            return xt::pyarray<bool>(self.isin(coords));
        }, "Check which particles are inside the box")
        
        // Operators (need to add these to the Box class)
        // .def("__eq__", [](const molcpp::Box& self, const molcpp::Box& other) {
        //     return operator==(self, other);
        // }, "Check box equality")
        // .def("__ne__", [](const molcpp::Box& self, const molcpp::Box& other) {
        //     return operator!=(self, other);
        // }, "Check box inequality")
        
        // Static factory methods
        .def_static("from_lengths_angles", &molcpp::Box::from_lengths_angles,
                   "Create box from lengths and angles", py::arg("lengths"), py::arg("angles"));
}
