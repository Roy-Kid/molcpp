#pragma once

#include "packer.hpp"
#include "constraint.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace molcpp::pack {

MolPacker::MolPacker() : rng_(std::random_device{}()) {}

void MolPacker::add_target(target_type&& target) {
    targets_.push_back(std::move(target));
}

MolPacker::target_type& MolPacker::def_target(
    const frame_type& frame,
    size_t number,
    const Constraint& constraint,
    bool is_fixed,
    const std::string& name) {
    target_type target(frame, number, constraint, is_fixed, name);
    targets_.push_back(std::move(target));
    return targets_.back();
}

MolPacker::frame_type MolPacker::pack(
    const std::vector<target_type>& targets,
    size_t max_steps,
    uint32_t seed) {
    const auto& target_list = targets.empty() ? targets_ : targets;
    if (target_list.empty()) {
        throw std::runtime_error("No targets to pack");
    }
    init_rng(seed);
    auto positions = generate_initial_positions();

    float current_penalty = calculate_total_penalty(positions);
    float previous_penalty = current_penalty;
    float learning_rate = 0.01f;
    for (size_t step = 0; step < max_steps; ++step) {
        bool improved = optimize_step(positions, current_penalty, learning_rate);
        if (is_converged(current_penalty, previous_penalty)) break;
        learning_rate = improved ? std::min(learning_rate * 1.1f, 0.1f)
                                 : std::max(learning_rate * 0.9f, 0.001f);
        previous_penalty = current_penalty;
    }
    return build_result_frame(positions);
}

size_t MolPacker::n_points() const {
    size_t total = 0;
    for (const auto& target : targets_) {
        total += target.n_points();
    }
    return total;
}

xt::xarray<float> MolPacker::points() const {
    if (targets_.empty()) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    
    std::vector<xt::xarray<float>> target_points;
    for (const auto& target : targets_) {
        auto points = target.points();
        if (points.size() > 0) {
            target_points.push_back(std::move(points));
        }
    }
    
    if (target_points.empty()) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    
    // Concatenate all points
    size_t total_points = 0;
    for (const auto& points : target_points) {
        total_points += points.shape(0);
    }
    
    auto result = xt::xarray<float>::from_shape({total_points, 3});
    size_t offset = 0;
    
    for (const auto& points : target_points) {
        size_t n_points = points.shape(0);
        for (size_t i = 0; i < n_points; ++i) {
            for (int dim = 0; dim < 3; ++dim) {
                result(offset + i, dim) = points(i, dim);
            }
        }
        offset += n_points;
    }
    
    return result;
}

void MolPacker::clear() {
    targets_.clear();
}

void MolPacker::init_rng(uint32_t seed) {
    if (seed == 0) {
        seed = std::random_device{}();
    }
    rng_.seed(seed);
}

xt::xarray<float> MolPacker::generate_initial_positions() {
    size_t total_points = n_points();
    if (total_points == 0) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    
    auto positions = xt::xarray<float>::from_shape({total_points, 3});
    size_t offset = 0;
    for (const auto& target : targets_) {
        const auto n = target.n_points();
        if (n == 0) { continue; }
        // Let the constraint sampler initialize if available
        auto sub = xt::xarray<float>::from_shape({n,3});
        if (target.get_constraint().sampler) {
            target.get_constraint().sampler(sub, rng_);
        } else {
            std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
            for (size_t i = 0; i < n; ++i) {
                sub(i,0) = dist(rng_); sub(i,1) = dist(rng_); sub(i,2) = dist(rng_);
            }
        }
        for (size_t i = 0; i < n; ++i) {
            positions(offset + i, 0) = sub(i,0);
            positions(offset + i, 1) = sub(i,1);
            positions(offset + i, 2) = sub(i,2);
        }
        offset += n;
    }
    return positions;
}

float MolPacker::calculate_total_penalty(const xt::xarray<float>& positions) const {
    float total_penalty = 0.0f;
    size_t offset = 0;
    for (const auto& target : targets_) {
        const auto n = target.n_points();
        if (n == 0) { continue; }
        auto sub = xt::xarray<float>::from_shape({n,3});
        for (size_t i = 0; i < n; ++i) {
            sub(i,0) = positions(offset + i, 0);
            sub(i,1) = positions(offset + i, 1);
            sub(i,2) = positions(offset + i, 2);
        }
        total_penalty += target.get_constraint().penalty(sub);
        offset += n;
    }
    return total_penalty;
}

xt::xarray<float> MolPacker::calculate_total_gradient(const xt::xarray<float>& positions) const {
    if (positions.size() == 0) {
        return xt::xarray<float>::from_shape({0, 3});
    }
    size_t n_points = positions.shape(0);
    auto total_grad = xt::xarray<float>::from_shape({n_points, 3});
    for (size_t i = 0; i < n_points; ++i) {
        total_grad(i,0) = total_grad(i,1) = total_grad(i,2) = 0.0f;
    }
    size_t offset = 0;
    for (const auto& target : targets_) {
        const auto n = target.n_points();
        if (n == 0) { continue; }
        auto sub = xt::xarray<float>::from_shape({n,3});
        for (size_t i = 0; i < n; ++i) {
            sub(i,0) = positions(offset + i, 0);
            sub(i,1) = positions(offset + i, 1);
            sub(i,2) = positions(offset + i, 2);
        }
        auto grad = target.get_constraint().dpenalty(sub);
        if (grad.size() > 0) {
            for (size_t i = 0; i < n; ++i) {
                total_grad(offset + i, 0) += grad(i,0);
                total_grad(offset + i, 1) += grad(i,1);
                total_grad(offset + i, 2) += grad(i,2);
            }
        }
        offset += n;
    }
    return total_grad;
}

bool MolPacker::optimize_step(xt::xarray<float>& positions, float& current_penalty, float learning_rate) {
    if (positions.size() == 0) return false;
    auto gradient = calculate_total_gradient(positions);
    auto new_positions = positions - learning_rate * gradient;
    float new_penalty = calculate_total_penalty(new_positions);
    if (new_penalty < current_penalty) {
        positions = std::move(new_positions);
        current_penalty = new_penalty;
        return true;
    }
    
    return false;
}

MolPacker::frame_type MolPacker::build_result_frame(const xt::xarray<float>& positions) const {
    frame_type result;
    
    if (positions.size() == 0) {
        return result;
    }
    
    size_t total_points = positions.shape(0);
    
    // Create atoms block
    molcpp::Block atoms;
    
    // Add coordinates in both formats for compatibility
    auto x = xt::xarray<float>::from_shape({total_points});
    auto y = xt::xarray<float>::from_shape({total_points});
    auto z = xt::xarray<float>::from_shape({total_points});
    
    for (size_t i = 0; i < total_points; ++i) {
        x(i) = positions(i, 0);
        y(i) = positions(i, 1);
        z(i) = positions(i, 2);
    }
    
    atoms["x"] = std::move(x);
    atoms["y"] = std::move(y);
    atoms["z"] = std::move(z);
    
    // Also add coords format for convenience
    auto coords = xt::xarray<float>::from_shape({total_points, 3});
    for (size_t i = 0; i < total_points; ++i) {
        coords(i, 0) = positions(i, 0);
        coords(i, 1) = positions(i, 1);
        coords(i, 2) = positions(i, 2);
    }
    atoms["coords"] = std::move(coords);
    
    // Add atom IDs
    auto ids = xt::xarray<float>::from_shape({total_points});
    for (size_t i = 0; i < total_points; ++i) {
    ids(i) = static_cast<float>(i + 1);
    }
    atoms["id"] = std::move(ids);
    
    // Add atom types (default to 1 for now)
    auto types = xt::xarray<float>::from_shape({total_points});
    for (size_t i = 0; i < total_points; ++i) {
    types(i) = 1.0f;
    }
    atoms["type"] = std::move(types);
    
    result["atoms"] = std::move(atoms);
    
    return result;
}

bool MolPacker::is_converged(float current_penalty, float previous_penalty, float tolerance) const {
    return std::abs(current_penalty - previous_penalty) < tolerance;
}

} // namespace molcpp::pack
