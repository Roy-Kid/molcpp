// locality.cpp
// Python bindings for molcpp locality module

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>

#include <xtensor-python/pyarray.hpp>

#include "molcpp/locality/AABBQuery.hpp"
#include "molcpp/locality/NeighborBond.hpp"
#include "molcpp/locality/NeighborQuery.hpp"
#include "molcpp/locality/AABB.hpp"
#include "molcpp/spatial/box.hpp"
#include "molcpp/types.hpp"

namespace py = pybind11;

void bind_locality(py::module_& m) {
    using namespace molcpp::locality;
    using namespace molcpp;
    
    // Create locality submodule
    py::module_ locality = m.def_submodule("locality", "Locality and neighbor finding functionality");
    
    // Bind NeighborBond class - complete interface
    py::class_<NeighborBond>(locality, "NeighborBond")
        .def(py::init<>())
        .def(py::init<unsigned int, unsigned int, float, float, const xt::pyarray<float>&>())
        .def(py::init<unsigned int, unsigned int, float, const xt::pyarray<float>&>())
        .def("getQueryPointIdx", &NeighborBond::getQueryPointIdx)
        .def("setQueryPointIdx", &NeighborBond::setQueryPointIdx)
        .def("getPointIdx", &NeighborBond::getPointIdx)
        .def("setPointIdx", &NeighborBond::setPointIdx)
        .def("getWeight", &NeighborBond::getWeight)
        .def("setWeight", &NeighborBond::setWeight)
        .def("getVector", &NeighborBond::getVector)
        .def("setVector", &NeighborBond::setVector)
        .def("getDistance", &NeighborBond::getDistance)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def("lessIdRefWeight", &NeighborBond::lessIdRefWeight)
        .def("lessAsTuple", &NeighborBond::lessAsTuple)
        .def("lessAsDistance", &NeighborBond::lessAsDistance);
    
    // Bind QueryArgs enum and struct
    py::enum_<QueryType>(locality, "QueryType")
        .value("none", QueryType::none)
        .value("ball", QueryType::ball)
        .value("nearest", QueryType::nearest);
    
    py::class_<QueryArgs>(locality, "QueryArgs")
        .def(py::init<>())
        .def(py::init([]() {
            QueryArgs args;
            args.mode = QueryType::ball;  // Default to ball mode
            return args;
        }))
        .def_readwrite("mode", &QueryArgs::mode)
        .def_readwrite("r_max", &QueryArgs::r_max)
        .def_readwrite("r_min", &QueryArgs::r_min)
        .def_readwrite("r_guess", &QueryArgs::r_guess)
        .def_readwrite("num_neighbors", &QueryArgs::num_neighbors)
        .def_readwrite("scale", &QueryArgs::scale)
        .def_readwrite("exclude_ii", &QueryArgs::exclude_ii);
    
    // Bind NeighborQuery base class - abstract class, no constructor
    py::class_<NeighborQuery>(locality, "NeighborQuery")
        .def("getNPoints", &NeighborQuery::getNPoints)
        .def("getBox", &NeighborQuery::getBox, py::return_value_policy::reference_internal)
        .def("getPoints", &NeighborQuery::getPoints, py::return_value_policy::reference_internal)
        .def("setBox", &NeighborQuery::setBox)
        .def("setPoints", &NeighborQuery::setPoints)
        .def("addPoints", &NeighborQuery::addPoints)
        .def("querySingle", &NeighborQuery::querySingle)
        .def("query", &NeighborQuery::query);
    
    // Bind AABBQuery class - concrete implementation
    py::class_<AABBQuery, NeighborQuery>(locality, "AABBQuery")
        .def(py::init<const Box&, const XYZ&>())
        .def("build", &AABBQuery::build)
        .def("update", &AABBQuery::update)
        .def("getAABBTree", &AABBQuery::getAABBTree, py::return_value_policy::reference_internal);
    
    // Bind AABB class - complete interface
    py::class_<AABB>(locality, "AABB")
        .def(py::init<>())
        .def(py::init<const xt::pyarray<float>&, const xt::pyarray<float>&>())
        .def(py::init<const xt::pyarray<float>&, float>())
        .def(py::init<const xt::pyarray<float>&, unsigned int>())
        .def("getPosition", &AABB::getPosition)
        .def("getLower", &AABB::getLower)
        .def("getUpper", &AABB::getUpper)
        .def("translate", &AABB::translate)
        .def_readwrite("tag", &AABB::tag);
    
    // Bind AABBSphere class - complete interface
    py::class_<AABBSphere>(locality, "AABBSphere")
        .def(py::init<>())
        .def(py::init<const xt::pyarray<float>&, float>())
        .def(py::init<const xt::pyarray<float>&, float, unsigned int>())
        .def("getPosition", &AABBSphere::getPosition)
        .def("translate", &AABBSphere::translate)
        .def_readwrite("radius", &AABBSphere::radius)
        .def_readwrite("tag", &AABBSphere::tag);
    
    // Bind free functions for AABB operations
    locality.def("overlap", py::overload_cast<const AABB&, const AABB&>(overlap), "Check if two AABBs overlap");
    locality.def("overlap", py::overload_cast<const AABB&, const AABBSphere&>(overlap), "Check if AABB and AABBSphere overlap");
    locality.def("contains", py::overload_cast<const AABB&, const AABB&>(contains), "Check if one AABB contains another");
    locality.def("contains", [](const AABB& aabb, const xt::pyarray<float>& point) -> bool {
        // Check if AABB contains a point
        auto lower = aabb.getLower();
        auto upper = aabb.getUpper();
        for (size_t i = 0; i < 3; ++i) {
            if (point[i] < lower[i] || point[i] > upper[i]) {
                return false;
            }
        }
        return true;
    }, "Check if AABB contains a point");
    locality.def("merge", merge, "Merge two AABBs");
}
