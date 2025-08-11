#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <xtensor-python/pyarray.hpp>
#include <xtensor-python/pytensor.hpp>

#include <molcpp/locality/AABB.hpp>
#include <molcpp/locality/AABBTree.hpp>
#include <molcpp/locality/AABBQuery.hpp>
#include <molcpp/locality/NeighborList.hpp>
#include <molcpp/locality/NeighborQuery.hpp>
#include <molcpp/box/Box.hpp>

namespace py = pybind11;

using namespace molcpp;
using namespace molcpp::locality;
using namespace molcpp::box;

PYBIND11_MODULE(molcpp_locality, m) {
    m.doc() = "Molcpp locality module - AABB-based neighbor finding";

    // xtensor-python import
    xt::import_numpy();

    // Box class
    py::class_<Box>(m, "Box")
        .def(py::init<>(), "Create a unit cubic box")
        .def(py::init<double, double, double, double, double, double, bool>(),
             py::arg("Lx"), py::arg("Ly"), py::arg("Lz"),
             py::arg("xy") = 0.0, py::arg("xz") = 0.0, py::arg("yz") = 0.0,
             py::arg("is_2d") = false,
             "Create a box with specified dimensions")
        .def_property_readonly("Lx", &Box::getLx, "Box length in x direction")
        .def_property_readonly("Ly", &Box::getLy, "Box length in y direction")
        .def_property_readonly("Lz", &Box::getLz, "Box length in z direction")
        .def_property_readonly("xy", &Box::getxy, "Tilt factor xy")
        .def_property_readonly("xz", &Box::getxz, "Tilt factor xz")
        .def_property_readonly("yz", &Box::getyz, "Tilt factor yz")
        .def_property_readonly("volume", &Box::getVolume, "Box volume")
        .def_property_readonly("is_2d", &Box::is2D, "Whether box is 2D")
        .def("get_vectors", &Box::getVectors, "Get box vectors as 3x3 matrix")
        .def("get_nearest_plane_distance", &Box::getNearestPlaneDistance,
             "Get nearest plane distances")
        .def("wrap", [](const Box& box, const xt::pyarray<double>& pos) {
            Vec3 vec_pos;
            for (size_t i = 0; i < 3; ++i) {
                vec_pos(i) = pos(i);
            }
            Vec3 wrapped = box.wrap(vec_pos);
            xt::pyarray<double> result = xt::zeros<double>({3});
            for (size_t i = 0; i < 3; ++i) {
                result(i) = wrapped(i);
            }
            return result;
        }, py::arg("position"), "Wrap position into box")
        .def("minimum_image", [](const Box& box, 
                                const xt::pyarray<double>& r_i,
                                const xt::pyarray<double>& r_j) {
            Vec3 vec_i, vec_j;
            for (size_t i = 0; i < 3; ++i) {
                vec_i(i) = r_i(i);
                vec_j(i) = r_j(i);
            }
            Vec3 dr = box.minimumImage(vec_i, vec_j);
            xt::pyarray<double> result = xt::zeros<double>({3});
            for (size_t i = 0; i < 3; ++i) {
                result(i) = dr(i);
            }
            return result;
        }, py::arg("r_i"), py::arg("r_j"), "Compute minimum image vector")
        .def("distance", [](const Box& box,
                           const xt::pyarray<double>& r_i,
                           const xt::pyarray<double>& r_j) {
            Vec3 vec_i, vec_j;
            for (size_t i = 0; i < 3; ++i) {
                vec_i(i) = r_i(i);
                vec_j(i) = r_j(i);
            }
            return box.distance(vec_i, vec_j);
        }, py::arg("r_i"), py::arg("r_j"), "Compute minimum image distance")
        .def("set_periodic", &Box::setPeriodic,
             py::arg("x"), py::arg("y"), py::arg("z"),
             "Set periodic boundary conditions");

    // QueryType enum
    py::enum_<QueryType>(m, "QueryType")
        .value("ball", QueryType::ball)
        .value("nearest", QueryType::nearest);

    // QueryArgs structure
    py::class_<QueryArgs>(m, "QueryArgs")
        .def(py::init<>())
        .def_readwrite("mode", &QueryArgs::mode)
        .def_readwrite("r_max", &QueryArgs::r_max)
        .def_readwrite("r_min", &QueryArgs::r_min)
        .def_readwrite("scale", &QueryArgs::scale)
        .def_readwrite("r_guess", &QueryArgs::r_guess)
        .def_readwrite("num_neighbors", &QueryArgs::num_neighbors)
        .def_readwrite("exclude_ii", &QueryArgs::exclude_ii)
        .def_static("ball", &QueryArgs::ball,
                   py::arg("r_max"), py::arg("r_min") = 0.0,
                   py::arg("exclude_ii") = false,
                   "Create arguments for ball query")
        .def_static("nearest", &QueryArgs::nearest,
                   py::arg("k"), py::arg("r_guess") = DEFAULT_R_GUESS,
                   py::arg("scale") = DEFAULT_SCALE, py::arg("exclude_ii") = true,
                   "Create arguments for k-nearest neighbor query");

    // NeighborBond structure
    py::class_<NeighborBond>(m, "NeighborBond")
        .def(py::init<>())
        .def(py::init<unsigned int, unsigned int, double>())
        .def_readwrite("query_point_idx", &NeighborBond::query_point_idx)
        .def_readwrite("point_idx", &NeighborBond::point_idx)
        .def_readwrite("distance", &NeighborBond::distance)
        .def_readwrite("weight", &NeighborBond::weight)
        .def_property("distance_vec",
            [](const NeighborBond& bond) {
                xt::pyarray<double> vec = xt::zeros<double>({3});
                for (size_t i = 0; i < 3; ++i) {
                    vec(i) = bond.distance_vec(i);
                }
                return vec;
            },
            [](NeighborBond& bond, const xt::pyarray<double>& vec) {
                for (size_t i = 0; i < 3; ++i) {
                    bond.distance_vec(i) = vec(i);
                }
            });

    // NeighborList class
    py::class_<NeighborList>(m, "NeighborList")
        .def(py::init<>())
        .def(py::init<size_t>(), py::arg("capacity"))
        .def("add_bond", py::overload_cast<const NeighborBond&>(&NeighborList::addBond))
        .def("add_bond", py::overload_cast<unsigned int, unsigned int, double>(
            &NeighborList::addBond))
        .def("size", &NeighborList::size)
        .def("empty", &NeighborList::empty)
        .def("clear", &NeighborList::clear)
        .def("reserve", &NeighborList::reserve)
        .def("sort", &NeighborList::sort)
        .def("is_sorted", &NeighborList::isSorted)
        .def("__len__", &NeighborList::size)
        .def("__getitem__", [](const NeighborList& nlist, size_t idx) {
            return nlist[idx];
        })
        .def("filter_r", &NeighborList::filterR,
             py::arg("r_max"), py::arg("r_min") = 0.0,
             "Filter bonds by distance range")
        .def("get_bonds", &NeighborList::getBonds, py::return_value_policy::reference_internal)
        .def_property_readonly("query_point_indices", 
            [](const NeighborList& nlist) { return nlist.getQueryPointIndices(); })
        .def_property_readonly("point_indices",
            [](const NeighborList& nlist) { return nlist.getPointIndices(); })
        .def_property_readonly("distances",
            [](const NeighborList& nlist) { return nlist.getDistances(); })
        .def_property_readonly("weights",
            [](const NeighborList& nlist) { return nlist.getWeights(); })
        .def_property_readonly("distance_vectors",
            [](const NeighborList& nlist) { return nlist.getDistanceVectors(); })
        .def("get_neighbors_for_point", &NeighborList::getNeighborsForPoint,
             py::arg("query_idx"), "Get neighbors for a specific query point")
        .def("get_neighbor_counts", &NeighborList::getNeighborCounts,
             py::arg("num_query_points"), "Get neighbor counts for each query point");

    // AABB structure
    py::class_<AABB>(m, "AABB")
        .def(py::init<>())
        .def(py::init([](const xt::pyarray<double>& lower, const xt::pyarray<double>& upper) {
            Vec3 vec_lower, vec_upper;
            for (size_t i = 0; i < 3; ++i) {
                vec_lower(i) = lower(i);
                vec_upper(i) = upper(i);
            }
            return AABB(vec_lower, vec_upper);
        }), py::arg("lower"), py::arg("upper"))
        .def(py::init([](const xt::pyarray<double>& position, double radius) {
            Vec3 vec_pos;
            for (size_t i = 0; i < 3; ++i) {
                vec_pos(i) = position(i);
            }
            return AABB(vec_pos, radius);
        }), py::arg("position"), py::arg("radius"))
        .def_property_readonly("position", [](const AABB& aabb) {
            Vec3 pos = aabb.getPosition();
            xt::pyarray<double> result = xt::zeros<double>({3});
            for (size_t i = 0; i < 3; ++i) {
                result(i) = pos(i);
            }
            return result;
        })
        .def_property_readonly("lower", [](const AABB& aabb) {
            const Vec3& lower = aabb.getLower();
            xt::pyarray<double> result = xt::zeros<double>({3});
            for (size_t i = 0; i < 3; ++i) {
                result(i) = lower(i);
            }
            return result;
        })
        .def_property_readonly("upper", [](const AABB& aabb) {
            const Vec3& upper = aabb.getUpper();
            xt::pyarray<double> result = xt::zeros<double>({3});
            for (size_t i = 0; i < 3; ++i) {
                result(i) = upper(i);
            }
            return result;
        })
        .def_readwrite("tag", &AABB::tag);

    // AABBTree class
    py::class_<AABBTree>(m, "AABBTree")
        .def(py::init<>())
        .def("build_tree", [](AABBTree& tree, const std::vector<AABB>& aabbs, bool parallelism) {
            tree.buildTree(aabbs, parallelism);
        }, py::arg("aabbs"), py::arg("parallelism") = true)
        .def("query", [](const AABBTree& tree, const AABB& aabb) {
            std::vector<unsigned int> result;
            tree.query(aabb, result);
            return result;
        }, py::arg("aabb"), "Query tree with AABB")
        .def_property_readonly("root", &AABBTree::getRoot)
        .def_property_readonly("num_nodes", &AABBTree::getNumNodes);

    // AABBQuery class
    py::class_<AABBQuery, NeighborQuery>(m, "AABBQuery")
        .def(py::init<>())
        .def(py::init<const Box&, const xt::pyarray<double>&>(),
             py::arg("box"), py::arg("points"))
        .def("build_tree", &AABBQuery::buildTree, "Build the AABB tree")
        .def("query", [](const AABBQuery& query, 
                        const xt::pyarray<double>& query_points,
                        const QueryArgs& args) {
            return query.query(query_points, args);
        }, py::arg("query_points"), py::arg("args"),
           "Query for neighbors")
        .def_property_readonly("box", &AABBQuery::getBox, py::return_value_policy::reference_internal)
        .def_property_readonly("points", &AABBQuery::getPoints, py::return_value_policy::reference_internal)
        .def_property_readonly("n_points", &AABBQuery::getNPoints);

    // Module-level functions
    m.def("overlap", py::overload_cast<const AABB&, const AABB&>(&overlap),
          py::arg("a"), py::arg("b"), "Check if two AABBs overlap");
    m.def("contains", &contains, py::arg("a"), py::arg("b"),
          "Check if AABB a contains AABB b");
    m.def("merge", &merge, py::arg("a"), py::arg("b"),
          "Merge two AABBs");
}