#pragma once

#include "../core/frame.hpp"
#include "constraint.hpp"
#include "../types.hpp"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>
#include <string>

namespace molcpp::pack {

/**
 * @brief Target for molecular packing (float-only, value semantics).
 */
class Target {
public:
    using frame_type = molcpp::Frame;
    using constraint_type = Constraint;

    Target(const frame_type& frame,
           size_t number,
           const constraint_type& constraint,
           bool is_fixed = false,
           const std::string& name = "");

    // Move only (frames can be large). Copy disabled.
    Target(const Target& other) = delete;
    Target& operator=(const Target& other) = delete;
    Target(Target&& other) noexcept;
    Target& operator=(Target&& other) noexcept;
    ~Target();

    // Accessors
    const frame_type& get_frame() const { return frame_; }
    frame_type& get_frame() { return frame_; }
    size_t get_number() const { return number_; }
    void set_number(size_t number) { number_ = number; }
    const constraint_type& get_constraint() const { return constraint_; }
    bool is_fixed() const { return is_fixed_; }
    void set_fixed(bool fixed) { is_fixed_ = fixed; }
    const std::string& get_name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    size_t n_points() const;
    xt::xarray<float> points() const;
    std::string to_string() const;

private:
    frame_type frame_;
    size_t number_;
    constraint_type constraint_; // stored by value
    bool is_fixed_;
    std::string name_;

    xt::xarray<float> extract_coordinates() const;
};

} // namespace molcpp::pack

// Include implementation
#include "target_impl.hpp"
