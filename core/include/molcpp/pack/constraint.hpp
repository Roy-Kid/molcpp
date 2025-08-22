#pragma once

#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xfixed.hpp>
#include <memory>
#include <vector>
#include "../types.hpp"

namespace molcpp::pack {

// Forward declarations
template<typename T>
class Region;

/**
 * @brief Base constraint class that provides penalty functions for optimization.
 * 
 * Constraints are used by MolPacker to evaluate the quality of molecular packing.
 * Each constraint provides a penalty value and gradient for optimization algorithms.
 */
template<typename T>
class Constraint {
public:
    virtual ~Constraint() = default;
    
    /**
     * @brief Calculate penalty value for given points.
     * 
     * @param points Array of points (N, 3) to evaluate
     * @return Penalty value (higher = worse)
     */
    virtual T penalty(const xt::xarray<T>& points) const = 0;
    
    /**
     * @brief Calculate gradient of penalty function.
     * 
     * @param points Array of points (N, 3) to evaluate
     * @return Gradient array (N, 3) with same shape as points
     */
    virtual xt::xarray<T> dpenalty(const xt::xarray<T>& points) const = 0;
    
    /**
     * @brief Logical AND of two constraints.
     */
    std::unique_ptr<Constraint<T>> operator&(const Constraint<T>& other) const;
    
    /**
     * @brief Logical OR of two constraints.
     */
    std::unique_ptr<Constraint<T>> operator|(const Constraint<T>& other) const;
};

/**
 * @brief Logical AND constraint - both constraints must be satisfied.
 */
template<typename T>
class AndConstraint : public Constraint<T> {
public:
    AndConstraint(std::unique_ptr<Constraint<T>> a, std::unique_ptr<Constraint<T>> b);
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;

private:
    std::unique_ptr<Constraint<T>> a_;
    std::unique_ptr<Constraint<T>> b_;
};

/**
 * @brief Logical OR constraint - at least one constraint must be satisfied.
 */
template<typename T>
class OrConstraint : public Constraint<T> {
public:
    OrConstraint(std::unique_ptr<Constraint<T>> a, std::unique_ptr<Constraint<T>> b);
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;

private:
    std::unique_ptr<Constraint<T>> a_;
    std::unique_ptr<Constraint<T>> b_;
};

/**
 * @brief Constraint that points must be inside a box region.
 */
template<typename T>
class InsideBoxConstraint : public Constraint<T> {
public:
    InsideBoxConstraint(const Vec3<T>& lengths, const Vec3<T>& origin = Vec3<T>{0, 0, 0});
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;
    
    /**
     * @brief Logical NOT - points must be outside the box.
     */
    std::unique_ptr<Constraint<T>> operator~() const;

private:
    Vec3<T> lengths_;
    Vec3<T> origin_;
    Vec3<T> upper_;
};

/**
 * @brief Constraint that points must be outside a box region.
 */
template<typename T>
class OutsideBoxConstraint : public Constraint<T> {
public:
    OutsideBoxConstraint(const Vec3<T>& origin, const Vec3<T>& lengths);
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;
    
    /**
     * @brief Logical NOT - points must be inside the box.
     */
    std::unique_ptr<Constraint<T>> operator~() const;

private:
    Vec3<T> origin_;
    Vec3<T> upper_;
};

/**
 * @brief Constraint that points must be inside a sphere region.
 */
template<typename T>
class InsideSphereConstraint : public Constraint<T> {
public:
    InsideSphereConstraint(T radius, const Vec3<T>& center = Vec3<T>{0, 0, 0});
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;
    
    /**
     * @brief Logical NOT - points must be outside the sphere.
     */
    std::unique_ptr<Constraint<T>> operator~() const;

private:
    T radius_;
    Vec3<T> center_;
};

/**
 * @brief Constraint that points must be outside a sphere region.
 */
template<typename T>
class OutsideSphereConstraint : public Constraint<T> {
public:
    OutsideSphereConstraint(T radius, const Vec3<T>& center = Vec3<T>{0, 0, 0});
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;
    
    /**
     * @brief Logical NOT - points must be inside the sphere.
     */
    std::unique_ptr<Constraint<T>> operator~() const;

private:
    T radius_;
    Vec3<T> center_;
};

/**
 * @brief Constraint that enforces minimum distance between all points.
 */
template<typename T>
class MinDistanceConstraint : public Constraint<T> {
public:
    explicit MinDistanceConstraint(T min_distance);
    
    T penalty(const xt::xarray<T>& points) const override;
    xt::xarray<T> dpenalty(const xt::xarray<T>& points) const override;

private:
    T min_distance_;
};

// Type aliases for common types
using Constraintf = Constraint<float>;
using Constraintd = Constraint<double>;

} // namespace molcpp::pack

// Include implementation
#include "constraint_impl.hpp"
