#pragma once

#include "../types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <optional>

namespace molcpp::pack {

/**
 * @brief Optimization result structure
 */
template<typename T = Real>
struct OptimizationResult {
    CoordArray<T> positions;           // Final optimized positions (N, 3)
    T final_penalty;                   // Final penalty value
    Index iterations;                   // Number of iterations performed
    bool converged;                    // Whether optimization converged
    std::string status;                // Status message
    std::optional<CoordArray<T>> trajectory; // Optional optimization trajectory
};

/**
 * @brief Optimization parameters
 */
template<typename T = Real>
struct OptimizationParams {
    Index max_iterations = 1000;       // Maximum number of iterations
    T tolerance = 1e-6;                // Convergence tolerance
    T learning_rate = 0.01;            // Learning rate for gradient descent
    uint32_t seed = 42;                // Random seed
    bool verbose = false;               // Verbose output
    bool save_trajectory = false;      // Save optimization trajectory
};

/**
 * @brief Penalty function signature
 */
template<typename T = Real>
using PenaltyFunction = std::function<T(const CoordArray<T>&)>;

/**
 * @brief Gradient function signature
 */
template<typename T = Real>
using GradientFunction = std::function<CoordArray<T>(const CoordArray<T>&)>;

/**
 * @brief Base optimizer interface
 */
template<typename T = Real>
class Optimizer {
public:
    virtual ~Optimizer() = default;
    
    /**
     * @brief Optimize positions using penalty and gradient functions
     * 
     * @param initial_positions Initial positions (N, 3)
     * @param penalty_fn Penalty function
     * @param gradient_fn Gradient function
     * @param params Optimization parameters
     * @return Optimization result
     */
    virtual OptimizationResult<T> optimize(
        const CoordArray<T>& initial_positions,
        PenaltyFunction<T> penalty_fn,
        GradientFunction<T> gradient_fn,
        const OptimizationParams<T>& params = {}
    ) = 0;
    
    /**
     * @brief Get optimizer name
     */
    virtual std::string name() const = 0;
    
    /**
     * @brief Get optimizer description
     */
    virtual std::string description() const = 0;
};

/**
 * @brief Gradient descent optimizer (placeholder implementation)
 */
template<typename T = Real>
class GradientDescentOptimizer : public Optimizer<T> {
public:
    OptimizationResult<T> optimize(
        const CoordArray<T>& initial_positions,
        PenaltyFunction<T> penalty_fn,
        GradientFunction<T> gradient_fn,
        const OptimizationParams<T>& params = {}
    ) override;
    
    std::string name() const override { return "GradientDescent"; }
    std::string description() const override { return "Basic gradient descent optimizer"; }
};

/**
 * @brief L-BFGS optimizer (placeholder implementation)
 */
template<typename T = Real>
class LBFGSOptimizer : public Optimizer<T> {
public:
    OptimizationResult<T> optimize(
        const CoordArray<T>& initial_positions,
        PenaltyFunction<T> penalty_fn,
        GradientFunction<T> gradient_fn,
        const OptimizationParams<T>& params = {}
    ) override;
    
    std::string name() const override { return "L-BFGS"; }
    std::string description() const override { return "Limited-memory BFGS optimizer"; }
};

/**
 * @brief Factory function for creating optimizers
 */
template<typename T = Real>
std::unique_ptr<Optimizer<T>> make_optimizer(const std::string& type);

} // namespace molcpp::pack
