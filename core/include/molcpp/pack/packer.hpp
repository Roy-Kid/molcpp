#pragma once

#include "target.hpp"
#include "../core/frame.hpp"
#include "../types.hpp"
#include <xtensor/containers/xarray.hpp>
#include <vector>
#include <random>

namespace molcpp::pack {

class MolPacker {
public:
    using target_type = Target;
    using frame_type = molcpp::Frame;

    MolPacker();
    ~MolPacker() = default;

    void add_target(target_type&& target);
    target_type& def_target(const frame_type& frame,
                           size_t number,
                           const Constraint& constraint,
                           bool is_fixed = false,
                           const std::string& name = "");

    frame_type pack(const std::vector<target_type>& targets = {},
                    size_t max_steps = 1000,
                    uint32_t seed = 0);

    size_t n_points() const;
    xt::xarray<float> points() const;
    const std::vector<target_type>& get_targets() const { return targets_; }
    void clear();

private:
    std::vector<target_type> targets_;
    std::mt19937 rng_;

    void init_rng(uint32_t seed);
    xt::xarray<float> generate_initial_positions();
    float calculate_total_penalty(const xt::xarray<float>& positions) const;
    xt::xarray<float> calculate_total_gradient(const xt::xarray<float>& positions) const;
    bool optimize_step(xt::xarray<float>& positions, float& current_penalty, float learning_rate);
    frame_type build_result_frame(const xt::xarray<float>& positions) const;
    bool is_converged(float current_penalty, float previous_penalty, float tolerance = 1e-6f) const;
};

} // namespace molcpp::pack

#include "packer_impl.hpp"
