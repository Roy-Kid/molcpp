#pragma once

#include "../core/frame.hpp"
#include "constraint.hpp"
#include "../types.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>
#include <memory>
#include <string>

namespace molcpp::pack {

// Forward declarations
template<typename T>
class Optimizer;

/**
 * @brief Target for molecular packing.
 * 
 * Represents a molecular structure that needs to be packed with specified
 * number of copies and spatial constraints.
 */
template<typename T>
class Target {
public:
    using frame_type = molcpp::Frame;
    using constraint_type = Constraint<T>;
    using optimizer_type = Optimizer<T>;
    using vec3_type = Vec3<T>;

    /**
     * @brief Construct a new Target.
     * 
     * @param frame Molecular structure frame
     * @param number Number of copies to pack
     * @param constraint Spatial constraint for placement
     * @param is_fixed Whether this target is fixed in space
     * @param optimizer Optional optimizer for this target
     * @param name Optional name for this target
     */
    Target(const frame_type& frame,
           size_t number,
           std::unique_ptr<constraint_type> constraint,
           bool is_fixed = false,
           std::unique_ptr<optimizer_type> optimizer = nullptr,
           const std::string& name = "");

    /**
     * @brief Copy constructor.
     */
    Target(const Target& other);

    /**
     * @brief Move constructor.
     */
    Target(Target&& other) noexcept;

    /**
     * @brief Copy assignment operator.
     */
    Target& operator=(const Target& other);

    /**
     * @brief Move assignment operator.
     */
    Target& operator=(Target&& other) noexcept;

    /**
     * @brief Destructor.
     */
    ~Target() = default;

    // Accessors
    const frame_type& get_frame() const { return frame_; }
    frame_type& get_frame() { return frame_; }
    
    size_t get_number() const { return number_; }
    void set_number(size_t number) { number_ = number; }
    
    const constraint_type* get_constraint() const { return constraint_.get(); }
    constraint_type* get_constraint() { return constraint_.get(); }
    
    bool is_fixed() const { return is_fixed_; }
    void set_fixed(bool fixed) { is_fixed_ = fixed; }
    
    const optimizer_type* get_optimizer() const { return optimizer_.get(); }
    optimizer_type* get_optimizer() { return optimizer_.get(); }
    
    const std::string& get_name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    /**
     * @brief Get the total number of points (atoms) for this target.
     */
    size_t n_points() const;

    /**
     * @brief Get all points (coordinates) for this target.
     * 
     * Returns the coordinates of all atoms, replicated for each copy.
     * The result is an array of shape (n_points, 3).
     */
    xt::xarray<T> points() const;

    /**
     * @brief String representation of the target.
     */
    std::string to_string() const;

private:
    frame_type frame_;
    size_t number_;
    std::unique_ptr<constraint_type> constraint_;
    bool is_fixed_;
    std::unique_ptr<optimizer_type> optimizer_;
    std::string name_;

    /**
     * @brief Extract coordinates from the frame.
     * 
     * Handles different coordinate formats (xyz, x/y/z, etc.)
     */
    xt::xarray<T> extract_coordinates() const;
};

// Type aliases for common types
using Targetf = Target<float>;
using Targetd = Target<double>;

} // namespace molcpp::pack

// Include implementation
#include "target_impl.hpp"
