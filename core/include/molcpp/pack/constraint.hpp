#pragma once

#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include "molcpp/types.hpp"

namespace molcpp::pack {

using Array = xt::xarray<float>;
using Vec3f = Vec3<float>;

// Value-based constraint: stores penalty/gradient and optional sampler
struct Constraint {
	using PenaltyFn = std::function<float(const Array&)>;
	using GradFn = std::function<Array(const Array&)>;
	using SamplerFn = std::function<void(Array& /*sub_positions*/, std::mt19937& /*rng*/)>;

	PenaltyFn penalty;
	GradFn dpenalty;
	SamplerFn sampler; // optional: initialize points consistent with this constraint
	std::string name;
};

// Factory functions for common constraints
Constraint make_inside_box(const Vec3f& lengths, const Vec3f& origin = Vec3f{0,0,0});
Constraint make_outside_box(const Vec3f& origin, const Vec3f& lengths);
Constraint make_inside_sphere(float radius, const Vec3f& center = Vec3f{0,0,0});
Constraint make_outside_sphere(float radius, const Vec3f& center = Vec3f{0,0,0});
Constraint make_min_distance(float min_distance);
Constraint make_inter_molecular_min_distance(float min_distance, std::size_t group_size);

// Combinators
Constraint operator&(const Constraint& a, const Constraint& b);
Constraint operator|(const Constraint& a, const Constraint& b);

} // namespace molcpp::pack

// Include implementation
#include "constraint_impl.hpp"
