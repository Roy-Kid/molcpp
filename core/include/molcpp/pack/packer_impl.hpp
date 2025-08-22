#pragma once

#include "packer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace molcpp::pack {

// MolPacker implementation
template<typename T>
MolPacker<T>::MolPacker() : rng_(std::random_device{}()) {}

template<typename T>
void MolPacker<T>::add_target(const target_type& target) {
    targets_.push_back(target);
}

template<typename T>
typename MolPacker<T>::target_type& MolPacker<T>::def_target(
    const frame_type& frame,
    size_t number,
    std::unique_ptr<constraint_type> constraint,
    bool is_fixed,
    const std::string& name) {
    
    target_type target(frame, number, std::move(constraint), is_fixed, nullptr, name);
    targets_.push_back(std::move(target));
    return targets_.back();
}

template<typename T>
typename MolPacker<T>::frame_type MolPacker<T>::pack(
    const std::vector<target_type>& targets,
    size_t max_steps,
    uint32_t seed) {
    
    // Use provided targets or internal targets
    const auto& target_list = targets.empty() ? targets_ : targets;
    
    if (target_list.empty()) {
        throw std::runtime_error("No targets to pack");
    }
    
    // Initialize random number generator
    init_rng(seed);
    
    // Generate initial positions
    auto positions = generate_initial_positions();
    
    // Use default optimizer if available
    if (default_optimizer_) {
        auto penalty_fn = [this](const xt::xarray<T>& pos) -> T {
            return calculate_total_penalty(pos);
        };
        
        auto gradient_fn = [this](const xt::xarray<T>& pos) -> xt::xarray<T> {
            return calculate_total_gradient(pos);
        };
        
        OptimizationParams<T> params;
        params.max_iterations = max_steps;
        params.seed = seed;
        
        last_result_ = default_optimizer_->optimize(positions, penalty_fn, gradient_fn, params);
        positions = last_result_.positions;
    } else {
        // Fallback to basic optimization
        T current_penalty = calculate_total_penalty(positions);
        T previous_penalty = current_penalty;
        T learning_rate = T(0.01);
        
        for (size_t step = 0; step < max_steps; ++step) {
            // Perform optimization step
            bool improved = optimize_step(positions, current_penalty, learning_rate);
            
            // Check convergence
            if (is_converged(current_penalty, previous_penalty)) {
                break;
            }
            
            // Update learning rate
            if (improved) {
                learning_rate = std::min(learning_rate * T(1.1), T(0.1));
            } else {
                learning_rate = std::max(learning_rate * T(0.9), T(0.001));
            }
            
            previous_penalty = current_penalty;
        }
        
        // Create result for compatibility
        last_result_.positions = positions;
        last_result_.final_penalty = current_penalty;
        last_result_.iterations = max_steps;
        last_result_.converged = is_converged(current_penalty, previous_penalty);
        last_result_.status = "Basic optimization completed";
    }
    
    // Build and return result frame
    return build_result_frame(positions);
}

template<typename T>
typename MolPacker<T>::frame_type MolPacker<T>::pack_with_params(
    const std::vector<target_type>& targets,
    const OptimizationParams<T>& params) {
    
    // Use provided targets or internal targets
    const auto& target_list = targets.empty() ? targets_ : targets;
    
    if (target_list.empty()) {
        throw std::runtime_error("No targets to pack");
    }
    
    // Initialize random number generator
    init_rng(params.seed);
    
    // Generate initial positions
    auto positions = generate_initial_positions();
    
    // Use default optimizer if available
    if (default_optimizer_) {
        auto penalty_fn = [this](const xt::xarray<T>& pos) -> T {
            return calculate_total_penalty(pos);
        };
        
        auto gradient_fn = [this](const xt::xarray<T>& pos) -> xt::xarray<T> {
            return calculate_total_gradient(pos);
        };
        
        last_result_ = default_optimizer_->optimize(positions, penalty_fn, gradient_fn, params);
        positions = last_result_.positions;
    } else {
        // Fallback to basic optimization with params
        T current_penalty = calculate_total_penalty(positions);
        T previous_penalty = current_penalty;
        T learning_rate = params.learning_rate;
        
        for (size_t step = 0; step < params.max_iterations; ++step) {
            // Perform optimization step
            bool improved = optimize_step(positions, current_penalty, learning_rate);
            
            // Check convergence
            if (std::abs(current_penalty - previous_penalty) < params.tolerance) {
                break;
            }
            
            // Update learning rate
            if (improved) {
                learning_rate = std::min(learning_rate * T(1.1), T(0.1));
            } else {
                learning_rate = std::max(learning_rate * T(0.9), T(0.001));
            }
            
            previous_penalty = current_penalty;
        }
        
        // Create result for compatibility
        last_result_.positions = positions;
        last_result_.final_penalty = current_penalty;
        last_result_.iterations = params.max_iterations;
        last_result_.converged = std::abs(current_penalty - previous_penalty) < params.tolerance;
        last_result_.status = "Basic optimization completed";
    }
    
    // Build and return result frame
    return build_result_frame(positions);
}

template<typename T>
void MolPacker<T>::set_default_optimizer(const std::string& optimizer_type) {
    default_optimizer_ = make_optimizer<T>(optimizer_type);
}

template<typename T>
size_t MolPacker<T>::n_points() const {
    size_t total = 0;
    for (const auto& target : targets_) {
        total += target.n_points();
    }
    return total;
}

template<typename T>
xt::xarray<T> MolPacker<T>::points() const {
    if (targets_.empty()) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    std::vector<xt::xarray<T>> target_points;
    for (const auto& target : targets_) {
        auto points = target.points();
        if (points.size() > 0) {
            target_points.push_back(std::move(points));
        }
    }
    
    if (target_points.empty()) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    // Concatenate all points
    size_t total_points = 0;
    for (const auto& points : target_points) {
        total_points += points.shape(0);
    }
    
    auto result = xt::xarray<T>::from_shape({total_points, 3});
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

template<typename T>
void MolPacker<T>::clear() {
    targets_.clear();
}

template<typename T>
void MolPacker<T>::init_rng(uint32_t seed) {
    if (seed == 0) {
        seed = std::random_device{}();
    }
    rng_.seed(seed);
}

template<typename T>
xt::xarray<T> MolPacker<T>::generate_initial_positions() {
    size_t total_points = n_points();
    if (total_points == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    auto positions = xt::xarray<T>::from_shape({total_points, 3});
    
    // For now, generate random positions in a reasonable range
    // In a more sophisticated implementation, this would use the constraints
    std::uniform_real_distribution<T> dist(-10.0, 10.0);
    
    for (size_t i = 0; i < total_points; ++i) {
        for (int dim = 0; dim < 3; ++dim) {
            positions(i, dim) = dist(rng_);
        }
    }
    
    return positions;
}

template<typename T>
T MolPacker<T>::calculate_total_penalty(const xt::xarray<T>& positions) const {
    T total_penalty = T(0);
    
    for (const auto& target : targets_) {
        if (target.get_constraint()) {
            total_penalty += target.get_constraint()->penalty(positions);
        }
    }
    
    return total_penalty;
}

template<typename T>
xt::xarray<T> MolPacker<T>::calculate_total_gradient(const xt::xarray<T>& positions) const {
    if (positions.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = positions.shape(0);
    auto total_grad = xt::xarray<T>::from_shape({n_points, 3});
    
    // Initialize to zero
    for (size_t i = 0; i < n_points; ++i) {
        for (int dim = 0; dim < 3; ++dim) {
            total_grad(i, dim) = T(0);
        }
    }
    
    // Accumulate gradients from all constraints
    for (const auto& target : targets_) {
        if (target.get_constraint()) {
            auto grad = target.get_constraint()->dpenalty(positions);
            if (grad.size() > 0) {
                total_grad += grad;
            }
        }
    }
    
    return total_grad;
}

template<typename T>
bool MolPacker<T>::optimize_step(xt::xarray<T>& positions, T& current_penalty, T learning_rate) {
    if (positions.size() == 0) return false;
    
    // Calculate gradient
    auto gradient = calculate_total_gradient(positions);
    
    // Apply gradient descent step
    auto new_positions = positions - learning_rate * gradient;
    
    // Calculate new penalty
    T new_penalty = calculate_total_penalty(new_positions);
    
    // Check if we improved
    if (new_penalty < current_penalty) {
        positions = std::move(new_positions);
        current_penalty = new_penalty;
        return true;
    }
    
    return false;
}

template<typename T>
typename MolPacker<T>::frame_type MolPacker<T>::build_result_frame(const xt::xarray<T>& positions) const {
    frame_type result;
    
    if (positions.size() == 0) {
        return result;
    }
    
    size_t total_points = positions.shape(0);
    
    // Create atoms block
    molcpp::Block atoms;
    
    // Add coordinates
    auto x = xt::xarray<T>::from_shape({total_points});
    auto y = xt::xarray<T>::from_shape({total_points});
    auto z = xt::xarray<T>::from_shape({total_points});
    
    for (size_t i = 0; i < total_points; ++i) {
        x(i) = positions(i, 0);
        y(i) = positions(i, 1);
        z(i) = positions(i, 2);
    }
    
    atoms["x"] = std::move(x);
    atoms["y"] = std::move(y);
    atoms["z"] = std::move(z);
    
    // Add atom IDs
    auto ids = xt::xarray<T>::from_shape({total_points});
    for (size_t i = 0; i < total_points; ++i) {
        ids(i) = static_cast<T>(i + 1);
    }
    atoms["id"] = std::move(ids);
    
    // Add atom types (default to 1 for now)
    auto types = xt::xarray<T>::from_shape({total_points});
    for (size_t i = 0; i < total_points; ++i) {
        types(i) = T(1);
    }
    atoms["type"] = std::move(types);
    
    result["atoms"] = std::move(atoms);
    
    return result;
}

template<typename T>
bool MolPacker<T>::is_converged(T current_penalty, T previous_penalty, T tolerance) const {
    return std::abs(current_penalty - previous_penalty) < tolerance;
}

} // namespace molcpp::pack
