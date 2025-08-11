#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>
#include <xtensor-python/pyarray.hpp>

#include "molcpp/types.hpp"
#include "molcpp/box.hpp"
#include "molcpp/neighbor_aabb.hpp"

namespace py = pybind11;

using namespace molcpp;

static std::vector<Vec3f> to_vec3f(const xt::pyarray<float>& arr) {
    if (arr.dimension() != 2 || arr.shape(1) != 3) {
        throw std::runtime_error("points must have shape (N, 3)");
    }
    std::vector<Vec3f> out(arr.shape(0));
    auto r = arr.unchecked<2>();
    for (std::size_t i = 0; i < arr.shape(0); ++i) {
        out[i] = Vec3f{r(i, 0), r(i, 1), r(i, 2)};
    }
    return out;
}

PYBIND11_MODULE(molcpp_aabb, m) {
    m.doc() = "molcpp AABB neighbor finder with xtensor and OpenMP";

    py::class_<Vec3f>(m, "Vec3f")
        .def(py::init<float, float, float>())
        .def_readwrite("x", &Vec3f::x)
        .def_readwrite("y", &Vec3f::y)
        .def_readwrite("z", &Vec3f::z);

    py::class_<Box>(m, "Box")
        .def(py::init<float, float, float, bool, bool, bool>(),
             py::arg("Lx"), py::arg("Ly"), py::arg("Lz"),
             py::arg("periodic_x") = true, py::arg("periodic_y") = true, py::arg("periodic_z") = true)
        .def("Lx", &Box::Lx)
        .def("Ly", &Box::Ly)
        .def("Lz", &Box::Lz);

    py::class_<AABBNeighborQuery>(m, "AABBNeighborQuery")
        .def(py::init<
                 Box,
                 const std::vector<Vec3f>&,
                 int>(),
             py::arg("box"), py::arg("points"), py::arg("leaf_size") = 8)
        .def("query",
             [](const AABBNeighborQuery& self, const xt::pyarray<float>& query_points, float r_max, bool exclude_ii) {
                 auto qp = to_vec3f(query_points);
                 auto [i, j, d] = self.query(qp, r_max, exclude_ii);
                 xt::pyarray<int> ai = xt::adapt(i, {i.size()});
                 xt::pyarray<int> aj = xt::adapt(j, {j.size()});
                 xt::pyarray<float> ad = xt::adapt(d, {d.size()});
                 return py::make_tuple(std::move(ai), std::move(aj), std::move(ad));
             },
             py::arg("query_points"), py::arg("r_max"), py::arg("exclude_ii") = false);
}