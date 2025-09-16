//#pragma once already in header

#include "target.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace molcpp::pack {

inline Target::Target(const frame_type& frame,
               size_t number,
               const constraint_type& constraint,
               bool is_fixed,
               const std::string& name)
    : frame_(frame)
    , number_(number)
    , constraint_(constraint)
    , is_fixed_(is_fixed)
    , name_(name) {
    if (number == 0) {
        throw std::invalid_argument("Target number must be greater than 0");
    }
}

inline Target::Target(Target&& other) noexcept
    : frame_(std::move(other.frame_))
    , number_(other.number_)
    , constraint_(other.constraint_)
    , is_fixed_(other.is_fixed_)
    , name_(std::move(other.name_)) {
    other.number_ = 0;
    other.is_fixed_ = false;
}

inline Target::~Target() = default;

inline Target& Target::operator=(Target&& other) noexcept {
    if (this != &other) {
        frame_ = std::move(other.frame_);
        number_ = other.number_;
        constraint_ = other.constraint_;
        is_fixed_ = other.is_fixed_;
        name_ = std::move(other.name_);
        other.number_ = 0;
        other.is_fixed_ = false;
    }
    return *this;
}

inline size_t Target::n_points() const {
    auto coords = extract_coordinates();
    return coords.shape(0) * number_;
}

inline xt::xarray<float> Target::points() const {
    auto coords = extract_coordinates();
    if (coords.size() == 0) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    size_t n_atoms = coords.shape(0);
    size_t total_points = n_atoms * number_;
    auto result = xt::xarray<float>::from_shape({total_points, 3});
    for (size_t copy = 0; copy < number_; ++copy) {
        for (size_t atom = 0; atom < n_atoms; ++atom) {
            size_t idx = copy * n_atoms + atom;
            result(idx,0) = coords(atom,0);
            result(idx,1) = coords(atom,1);
            result(idx,2) = coords(atom,2);
        }
    }
    return result;
}

inline std::string Target::to_string() const {
    std::ostringstream oss;
    size_t n_atoms = 0;
    if (frame_.contains_block("atoms")) {
        const auto& atoms = frame_["atoms"];
        if (atoms.contains("id")) {
            xt::xarray<int> id_array = atoms["id"];
            n_atoms = id_array.size();
        } else if (atoms.contains("x")) {
            xt::xarray<float> x_array = atoms["x"];
            n_atoms = x_array.size();
        }
    }
    oss << "<Target " << name_ << ": " << n_atoms << " atoms, number=" << number_ << ">";
    return oss.str();
}

inline xt::xarray<float> Target::extract_coordinates() const {
    if (!frame_.contains_block("atoms")) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    const auto& atoms = frame_["atoms"];
    if (atoms.contains("coords")) {
        return atoms["coords"];
    } else if (atoms.contains("xyz")) {
        return atoms["xyz"];
    } else if (atoms.contains("x") && atoms.contains("y") && atoms.contains("z")) {
        xt::xarray<float> x = atoms["x"];
        xt::xarray<float> y = atoms["y"];
        xt::xarray<float> z = atoms["z"];
        size_t n_atoms = x.size();
        if (y.size() != n_atoms || z.size() != n_atoms) {
            throw std::runtime_error("Coordinate arrays x, y, z must have the same length");
        }
        auto coords = xt::xarray<float>::from_shape({n_atoms, 3});
        for (size_t i = 0; i < n_atoms; ++i) {
            coords(i,0) = x(i);
            coords(i,1) = y(i);
            coords(i,2) = z(i);
        }
        return coords;
    } else {
        return xt::xarray<float>::from_shape({0, 3});
    }
}

} // namespace molcpp::pack
