#pragma once

#include "target.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace molcpp::pack {

// Target implementation
template<typename T>
Target<T>::Target(const frame_type& frame,
                  size_t number,
                  std::unique_ptr<constraint_type> constraint,
                  bool is_fixed,
                  std::unique_ptr<optimizer_type> optimizer,
                  const std::string& name)
    : frame_(frame)
    , number_(number)
    , constraint_(std::move(constraint))
    , is_fixed_(is_fixed)
    , optimizer_(std::move(optimizer))
    , name_(name) {
    
    if (number == 0) {
        throw std::invalid_argument("Target number must be greater than 0");
    }
    
    if (!constraint_) {
        throw std::invalid_argument("Target must have a constraint");
    }
}

template<typename T>
Target<T>::Target(const Target& other)
    : frame_(other.frame_)
    , number_(other.number_)
    , constraint_(other.constraint_ ? other.constraint_->clone() : nullptr)
    , is_fixed_(other.is_fixed_)
    , optimizer_(other.optimizer_ ? other.optimizer_->clone() : nullptr)
    , name_(other.name_) {}

template<typename T>
Target<T>::Target(Target&& other) noexcept
    : frame_(std::move(other.frame_))
    , number_(other.number_)
    , constraint_(std::move(other.constraint_))
    , is_fixed_(other.is_fixed_)
    , optimizer_(std::move(other.optimizer_))
    , name_(std::move(other.name_)) {
    
    other.number_ = 0;
    other.is_fixed_ = false;
}

template<typename T>
Target<T>& Target<T>::operator=(const Target& other) {
    if (this != &other) {
        frame_ = other.frame_;
        number_ = other.number_;
        constraint_ = other.constraint_ ? other.constraint_->clone() : nullptr;
        is_fixed_ = other.is_fixed_;
        optimizer_ = other.optimizer_ ? other.optimizer_->clone() : nullptr;
        name_ = other.name_;
    }
    return *this;
}

template<typename T>
Target<T>& Target<T>::operator=(Target&& other) noexcept {
    if (this != &other) {
        frame_ = std::move(other.frame_);
        number_ = other.number_;
        constraint_ = std::move(other.constraint_);
        is_fixed_ = other.is_fixed_;
        optimizer_ = std::move(other.optimizer_);
        name_ = std::move(other.name_);
        
        other.number_ = 0;
        other.is_fixed_ = false;
    }
    return *this;
}

template<typename T>
size_t Target<T>::n_points() const {
    auto coords = extract_coordinates();
    return coords.shape(0) * number_;
}

template<typename T>
xt::xarray<T> Target<T>::points() const {
    auto coords = extract_coordinates();
    
    if (coords.size() == 0) {
        // Return empty array with correct shape for empty frames
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_atoms = coords.shape(0);
    size_t total_points = n_atoms * number_;
    
    // Create result array
    auto result = xt::xarray<T>::from_shape({total_points, 3});
    
    // Replicate coordinates for each copy
    for (size_t copy = 0; copy < number_; ++copy) {
        for (size_t atom = 0; atom < n_atoms; ++atom) {
            size_t result_idx = copy * n_atoms + atom;
            for (int dim = 0; dim < 3; ++dim) {
                result(result_idx, dim) = coords(atom, dim);
            }
        }
    }
    
    return result;
}

template<typename T>
std::string Target<T>::to_string() const {
    std::ostringstream oss;
    
    // Get atom count
    size_t n_atoms = 0;
    if (frame_.contains_block("atoms")) {
        const auto& atoms = frame_["atoms"];
        if (atoms.contains("id")) {
            xt::xarray<int> id_array = atoms["id"];
            n_atoms = id_array.size();
        } else if (atoms.contains("x")) {
            xt::xarray<double> x_array = atoms["x"];
            n_atoms = x_array.size();
        }
    }
    
    oss << "<Target " << name_ << ": " << n_atoms << " atoms in ";
    
    if (constraint_) {
        // For now, just show the constraint type
        oss << "constraint";
    } else {
        oss << "no constraint";
    }
    
    oss << ">";
    return oss.str();
}

template<typename T>
xt::xarray<T> Target<T>::extract_coordinates() const {
    if (!frame_.contains_block("atoms")) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    const auto& atoms = frame_["atoms"];
    
    // Try to get coordinates from different formats
    if (atoms.contains("coords")) {
        // coords format: (N, 3) array
        return atoms["coords"];
    } else if (atoms.contains("xyz")) {
        // xyz format: (N, 3) array
        return atoms["xyz"];
    } else if (atoms.contains("x") && atoms.contains("y") && atoms.contains("z")) {
        // x, y, z format: separate arrays
        xt::xarray<double> x = atoms["x"];
        xt::xarray<double> y = atoms["y"];
        xt::xarray<double> z = atoms["z"];
        
        size_t n_atoms = x.size();
        if (y.size() != n_atoms || z.size() != n_atoms) {
            throw std::runtime_error("Coordinate arrays x, y, z must have the same length");
        }
        
        // Stack coordinates
        auto coords = xt::xarray<T>::from_shape({n_atoms, 3});
        for (size_t i = 0; i < n_atoms; ++i) {
            coords(i, 0) = x(i);
            coords(i, 1) = y(i);
            coords(i, 2) = z(i);
        }
        
        return coords;
    } else {
        // No valid coordinate format found
        return xt::xarray<T>::from_shape({0, 3});
    }
}

} // namespace molcpp::pack
