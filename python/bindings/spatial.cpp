// spatial.cpp
// Python bindings for molcpp spatial module

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <xtensor-python/pyarray.hpp>

#include "molcpp/spatial/box.hpp"
#include "molcpp/types.hpp"

namespace py = pybind11;

void bind_spatial(py::module_& m) {
    using namespace molcpp;
    
    // Create spatial submodule
    py::module_ spatial = m.def_submodule("spatial", "Spatial geometry and boundaries");
    
    // Bind Box class
    py::class_<Box>(spatial, "Box")
        .def(py::init<const xt::pyarray<float>&, const xt::pyarray<float>&, const xt::pyarray<bool>&>())
        .def("matrix", &Box::matrix, py::return_value_policy::reference_internal)
        .def("origin", &Box::origin, py::return_value_policy::reference_internal)
        .def("pbc", &Box::pbc, py::return_value_policy::reference_internal)
        .def("getVolume", &Box::getVolume)
        .def("getNearestPlaneDistance", &Box::getNearestPlaneDistance)
        .def("getLatticeVector", &Box::getLatticeVector)
        .def("toFrac", &Box::toFrac)
        .def("toCart", &Box::toCart)
        .def("wrap", &Box::wrap)
        .def("delta", &Box::delta)
        .def_static("cube", &Box::cube)
        .def_static("orthorhombic", &Box::orthorhombic);
}
