// spatial.cpp
// Python bindings for molcpp spatial classes

#include "bindings.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
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

void bind_spatial(py::module_& m) {
    // Spatial module
    py::module_ spatial = m.def_submodule("spatial", "Spatial structures and boundaries");
    
    // Box class with numpy interface
    py::class_<molcpp::Box>(spatial, "Box")
        .def(py::init<>(), "Create a default box")
        .def(py::init<const Mat3&>(), "Create box from matrix")
        
        // Factory methods using numpy arrays
        .def_static("from_lengths_angles", 
            [](const xt::pyarray<double>& lengths, const xt::pyarray<double>& angles) {
                return molcpp::Box::from_lengths_angles(
                    pyarray_to_vec3(lengths), pyarray_to_vec3(angles));
            }, "Create box from lengths and angles as numpy arrays")
        
        // Setters using numpy arrays
        .def("set_lengths", 
            [](molcpp::Box& box, const xt::pyarray<double>& lengths) {
                box.set_lengths(pyarray_to_vec3(lengths));
            }, "Set box lengths from numpy array")
            
        .def("set_angles", 
            [](molcpp::Box& box, const xt::pyarray<double>& angles) {
                box.set_angles(pyarray_to_vec3(angles));
            }, "Set box angles from numpy array")
            
        .def("set_lengths_angles", 
            [](molcpp::Box& box, const xt::pyarray<double>& lengths, const xt::pyarray<double>& angles) {
                box.set_lengths_angles(pyarray_to_vec3(lengths), pyarray_to_vec3(angles));
            }, "Set both lengths and angles from numpy arrays")
        
        // Getters returning numpy arrays
        .def("get_lengths", 
            [](const molcpp::Box& box) -> xt::pyarray<double> {
                return vec3_to_pyarray(box.get_lengths());
            }, "Get box lengths as numpy array")
            
        .def("get_angles", 
            [](const molcpp::Box& box) -> xt::pyarray<double> {
                return vec3_to_pyarray(box.get_angles());
            }, "Get box angles as numpy array")
            
        .def("get_matrix", 
            [](const molcpp::Box& box) -> xt::pyarray<double> {
                return xt::pyarray<double>(box.get_matrix());
            }, "Get box matrix as numpy array")
        
        .def("__repr__", [](const molcpp::Box& box) {
            return "<molcpp.spatial.Box>";
        });
    
    // OrthogonalBoundary with numpy interface
    py::class_<molcpp::OrthogonalBoundary>(spatial, "OrthogonalBoundary")
        .def(py::init([](const xt::pyarray<double>& box_lengths) {
            return molcpp::OrthogonalBoundary(pyarray_to_vec3(box_lengths));
        }), py::arg("box_lengths"), 
        "Create orthogonal boundary from numpy array")
        
        .def(py::init([](const xt::pyarray<double>& box_lengths, bool px, bool py, bool pz) {
            std::array<bool, 3> periodic = {px, py, pz};
            return molcpp::OrthogonalBoundary(pyarray_to_vec3(box_lengths), periodic);
        }), py::arg("box_lengths"), py::arg("px") = true, py::arg("py") = true, py::arg("pz") = true,
        "Create orthogonal boundary from numpy array with custom periodicity")
        
        .def("get_box_lengths", 
            [](const molcpp::OrthogonalBoundary& boundary) -> xt::pyarray<double> {
                return vec3_to_pyarray(boundary.get_box_lengths());
            }, "Get box lengths as numpy array")
        
        .def("__repr__", [](const molcpp::OrthogonalBoundary&) {
            return "<molcpp.spatial.OrthogonalBoundary>";
        });
    
    // SphericalBoundary with numpy interface
    py::class_<molcpp::SphericalBoundary>(spatial, "SphericalBoundary")
        .def(py::init([](const xt::pyarray<double>& center, double radius, bool periodic) {
            return molcpp::SphericalBoundary(pyarray_to_vec3(center), radius, periodic);
        }), py::arg("center"), py::arg("radius"), py::arg("periodic") = false,
        "Create spherical boundary from numpy array center")
        
        .def("get_center", 
            [](const molcpp::SphericalBoundary& boundary) -> xt::pyarray<double> {
                return vec3_to_pyarray(boundary.get_center());
            }, "Get center as numpy array")
        
        .def("get_radius", &molcpp::SphericalBoundary::get_radius, "Get radius")
        
        .def("__repr__", [](const molcpp::SphericalBoundary&) {
            return "<molcpp.spatial.SphericalBoundary>";
        });
}
