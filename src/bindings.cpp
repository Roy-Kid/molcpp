#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>
#include <xtensor-python/pyarray.hpp>

#include "molcpp/nl_types.hpp"
#include "molcpp/AABBQuery.hpp"

namespace py = pybind11;

using namespace molcpp;

PYBIND11_MODULE(molcpp_aabb, m) {
    m.doc() = "molcpp AABB neighbor finder with xtensor and OpenMP";

    py::class_<AABBQuery>(m, "AABBQuery")
        .def(py::init([](const xt::pyarray<float>& points, py::object box, int leaf_size) {
                if (points.dimension() != 2 || points.shape(1) != 3) {
                    throw std::runtime_error("points must have shape (N, 3)");
                }
                xt::xtensor<float, 2> pts = points;
                struct PyBoxShim {
                    py::object obj;
                    float Lx() const { return obj.attr("Lx")().cast<float>(); }
                    float Ly() const { return obj.attr("Ly")().cast<float>(); }
                    float Lz() const { return obj.attr("Lz")().cast<float>(); }
                    bool periodic_x() const { return obj.attr("periodic_x")().cast<bool>(); }
                    bool periodic_y() const { return obj.attr("periodic_y")().cast<bool>(); }
                    bool periodic_z() const { return obj.attr("periodic_z")().cast<bool>(); }
                };
                PyBoxShim shim{box};
                return new AABBQuery(pts, shim, leaf_size);
            }),
            py::arg("points"), py::arg("box"), py::arg("leaf_size") = 8)
        .def("query",
             [](const AABBQuery& self, const xt::pyarray<float>& query_points, float r_max, bool exclude_ii) {
                 xt::xtensor<float, 2> qp = query_points;
                 auto [i, j, d] = self.query(qp, r_max, exclude_ii);
                 return py::make_tuple(py::cast(i), py::cast(j), py::cast(d));
             },
             py::arg("query_points"), py::arg("r_max"), py::arg("exclude_ii") = false);
}