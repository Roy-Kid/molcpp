#pragma once

#include "target.hpp"
#include "optimizer.hpp"
#include "../core/frame.hpp"
#include "../types.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>
#include <vector>
#include <random>
#include <memory>

namespace molcpp::pack {

/**
 * @brief Core molecular packing algorithm.
 * 
 * Implements the main packing logic using constraint-based optimization.
 * This is a native C++ implementation that doesn't rely on external tools.
 */
template<typename T = Real>
class MolPacker {
public:
    using target_type = Target<T>;
    using frame_type = molcpp::Frame;
    using vec3_type = Vec3<T>;
    using constraint_type = Constraint<T>;

    /**
     * @brief Construct a new MolPacker.
     */
    MolPacker();

    /**
     * @brief Destructor.
     */
    ~MolPacker() = default;

    /**
     * @brief Add a target to the packer.
     * 
     * @param target The target to add
     */
    void add_target(const target_type& target);

    /**
     * @brief Define a target directly.
     * 
     * @param frame Molecular structure frame
     * @param number Number of copies to pack
     * @param constraint Spatial constraint for placement
     * @param is_fixed Whether this target is fixed in space
     * @param name Optional name for this target
     * @return Reference to the created target
     */
    target_type& def_target(const frame_type& frame,
                           size_t number,
                           std::unique_ptr<constraint_type> constraint,
                           bool is_fixed = false,
                           const std::string& name = "");

    /**
     * @brief Pack all targets according to their constraints.
     * 
     * @param targets Optional list of targets (if not provided, uses internal targets)
     * @param max_steps Maximum optimization steps
     * @param seed Random seed for optimization
     * @return Packed molecular system as a Frame
     */
    frame_type pack(const std::vector<target_type>& targets = {},
                    size_t max_steps = 1000,
                    uint32_t seed = 0);
    
    /**
     * @brief Pack with custom optimization parameters
     * 
     * @param targets Optional list of targets
     * @param params Optimization parameters
     * @return Packed molecular system as a Frame
     */
    frame_type pack_with_params(const std::vector<target_type>& targets = {},
                               const OptimizationParams<T>& params = {});
    
    /**
     * @brief Set default optimizer for all targets
     * 
     * @param optimizer_type Type of optimizer ("gradient_descent", "lbfgs")
     */
    void set_default_optimizer(const std::string& optimizer_type);
    
    /**
     * @brief Get optimization statistics from last packing operation
     */
    const OptimizationResult<T>& get_last_result() const { return last_result_; }

    /**
     * @brief Get the total number of points across all targets.
     */
    size_t n_points() const;

    /**
     * @brief Get all points from all targets.
     * 
     * Returns concatenated coordinates from all targets.
     */
    CoordArray<T> points() const;

    /**
     * @brief Get the list of targets.
     */
    const std::vector<target_type>& get_targets() const { return targets_; }

    /**
     * @brief Clear all targets.
     */
    void clear();

private:
    std::vector<target_type> targets_;
    std::mt19937 rng_;
    std::unique_ptr<Optimizer<T>> default_optimizer_;
    OptimizationResult<T> last_result_;

    /**
     * @brief Initialize random number generator.
     */
    void init_rng(uint32_t seed);

    /**
     * @brief Generate initial positions for all targets.
     */
    CoordArray<T> generate_initial_positions();

    /**
     * @brief Calculate total penalty for all constraints.
     */
    T calculate_total_penalty(const CoordArray<T>& positions) const;

    /**
     * @brief Calculate gradient of total penalty.
     */
    CoordArray<T> calculate_total_gradient(const CoordArray<T>& positions) const;

    /**
     * @brief Perform one optimization step.
     */
    bool optimize_step(CoordArray<T>& positions, T& current_penalty, T learning_rate);

    /**
     * @brief Build final frame from optimized positions.
     */
    frame_type build_result_frame(const CoordArray<T>& positions) const;

    /**
     * @brief Check if optimization has converged.
     */
    bool is_converged(T current_penalty, T previous_penalty, T tolerance = 1e-6) const;
};


// Type aliases for common types
using MolPackerf = MolPacker<float>;
using MolPackerd = MolPacker<double>;
using DefaultMolPacker = MolPacker<>;  // Default to Real (float unless MOLCPP_USE_DOUBLE is set)

} // namespace molcpp::pack

// Include implementation
#include "packer_impl.hpp"
