#pragma once

#include "constraint.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace molcpp::pack {

// Constraint base class operators
template<typename T>
std::unique_ptr<Constraint<T>> Constraint<T>::operator&(const Constraint<T>& other) const {
    return std::make_unique<AndConstraint<T>>(
        std::make_unique<Constraint<T>>(*this),
        std::make_unique<Constraint<T>>(other)
    );
}

template<typename T>
std::unique_ptr<Constraint<T>> Constraint<T>::operator|(const Constraint<T>& other) const {
    return std::make_unique<OrConstraint<T>>(
        std::make_unique<Constraint<T>>(*this),
        std::make_unique<Constraint<T>>(other)
    );
}

// AndConstraint implementation
template<typename T>
AndConstraint<T>::AndConstraint(std::unique_ptr<Constraint<T>> a, std::unique_ptr<Constraint<T>> b)
    : a_(std::move(a)), b_(std::move(b)) {}

template<typename T>
T AndConstraint<T>::penalty(const xt::xarray<T>& points) const {
    return a_->penalty(points) + b_->penalty(points);
}

template<typename T>
xt::xarray<T> AndConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    auto grad_a = a_->dpenalty(points);
    auto grad_b = b_->dpenalty(points);
    return grad_a + grad_b;
}

// OrConstraint implementation
template<typename T>
OrConstraint<T>::OrConstraint(std::unique_ptr<Constraint<T>> a, std::unique_ptr<Constraint<T>> b)
    : a_(std::move(a)), b_(std::move(b)) {}

template<typename T>
T OrConstraint<T>::penalty(const xt::xarray<T>& points) const {
    T pa = a_->penalty(points);
    T pb = b_->penalty(points);
    return std::min(pa, pb);
}

template<typename T>
xt::xarray<T> OrConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    T pa = a_->penalty(points);
    T pb = b_->penalty(points);
    if (pa < pb) {
        return a_->dpenalty(points);
    } else {
        return b_->dpenalty(points);
    }
}

// InsideBoxConstraint implementation
template<typename T>
InsideBoxConstraint<T>::InsideBoxConstraint(const Vec3<T>& lengths, const Vec3<T>& origin)
    : lengths_(lengths), origin_(origin), upper_(origin + lengths) {}

template<typename T>
T InsideBoxConstraint<T>::penalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) return T(0);
    
    T penalty = T(0);
    size_t n_points = points.shape(0);
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        
        // Check if point is outside the box
        bool outside = false;
        for (int dim = 0; dim < 3; ++dim) {
            if (point[dim] < origin_[dim] || point[dim] > upper_[dim]) {
                outside = true;
                break;
            }
        }
        
        if (outside) {
            penalty += T(1);
        }
    }
    
    return penalty;
}

template<typename T>
xt::xarray<T> InsideBoxConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = points.shape(0);
    auto grad = xt::xarray<T>::from_shape({n_points, 3});
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        
        // Check if point is outside the box
        bool outside = false;
        for (int dim = 0; dim < 3; ++dim) {
            if (point[dim] < origin_[dim] || point[dim] > upper_[dim]) {
                outside = true;
                break;
            }
        }
        
        if (outside) {
            // Calculate gradient for points outside the box
            for (int dim = 0; dim < 3; ++dim) {
                if (point[dim] < origin_[dim]) {
                    grad(i, dim) = T(1);  // Push towards inside
                } else if (point[dim] > upper_[dim]) {
                    grad(i, dim) = T(-1); // Push towards inside
                } else {
                    grad(i, dim) = T(0);
                }
            }
        } else {
            // No gradient for points inside the box
            grad(i, 0) = T(0);
            grad(i, 1) = T(0);
            grad(i, 2) = T(0);
        }
    }
    
    return grad;
}

template<typename T>
std::unique_ptr<Constraint<T>> InsideBoxConstraint<T>::operator~() const {
    return std::make_unique<OutsideBoxConstraint<T>>(origin_, lengths_);
}

// OutsideBoxConstraint implementation
template<typename T>
OutsideBoxConstraint<T>::OutsideBoxConstraint(const Vec3<T>& origin, const Vec3<T>& lengths)
    : origin_(origin), upper_(origin + lengths) {}

template<typename T>
T OutsideBoxConstraint<T>::penalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) return T(0);
    
    T penalty = T(0);
    size_t n_points = points.shape(0);
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        
        // Check if point is inside the box
        bool inside = true;
        for (int dim = 0; dim < 3; ++dim) {
            if (point[dim] < origin_[dim] || point[dim] > upper_[dim]) {
                inside = false;
                break;
            }
        }
        
        if (inside) {
            penalty += T(1);
        }
    }
    
    return penalty;
}

template<typename T>
xt::xarray<T> OutsideBoxConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = points.shape(0);
    auto grad = xt::xarray<T>::from_shape({n_points, 3});
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        
        // Check if point is inside the box
        bool inside = true;
        for (int dim = 0; dim < 3; ++dim) {
            if (point[dim] < origin_[dim] || point[dim] > upper_[dim]) {
                inside = false;
                break;
            }
        }
        
        if (inside) {
            // Calculate gradient for points inside the box
            for (int dim = 0; dim < 3; ++dim) {
                T dist_to_origin = std::abs(point[dim] - origin_[dim]);
                T dist_to_upper = std::abs(point[dim] - upper_[dim]);
                
                if (dist_to_origin < dist_to_upper) {
                    grad(i, dim) = T(-1); // Push towards origin
                } else {
                    grad(i, dim) = T(1);  // Push towards upper
                }
            }
        } else {
            // No gradient for points outside the box
            grad(i, 0) = T(0);
            grad(i, 1) = T(0);
            grad(i, 2) = T(0);
        }
    }
    
    return grad;
}

template<typename T>
std::unique_ptr<Constraint<T>> OutsideBoxConstraint<T>::operator~() const {
    return std::make_unique<InsideBoxConstraint<T>>(upper_ - origin_, origin_);
}

// InsideSphereConstraint implementation
template<typename T>
InsideSphereConstraint<T>::InsideSphereConstraint(T radius, const Vec3<T>& center)
    : radius_(radius), center_(center) {}

template<typename T>
T InsideSphereConstraint<T>::penalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) return T(0);
    
    T penalty = T(0);
    size_t n_points = points.shape(0);
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        Vec3<T> diff = point - center_;
        T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
        
        if (distance > radius_) {
            penalty += T(1);
        }
    }
    
    return penalty;
}

template<typename T>
xt::xarray<T> InsideSphereConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = points.shape(0);
    auto grad = xt::xarray<T>::from_shape({n_points, 3});
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        Vec3<T> diff = point - center_;
        T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
        
        if (distance > radius_) {
            // Push point towards center
            if (distance > T(0)) {
                grad(i, 0) = -diff[0] / distance;
                grad(i, 1) = -diff[1] / distance;
                grad(i, 2) = -diff[2] / distance;
            } else {
                grad(i, 0) = T(0);
                grad(i, 1) = T(0);
                grad(i, 2) = T(0);
            }
        } else {
            // No gradient for points inside the sphere
            grad(i, 0) = T(0);
            grad(i, 1) = T(0);
            grad(i, 2) = T(0);
        }
    }
    
    return grad;
}

template<typename T>
std::unique_ptr<Constraint<T>> InsideSphereConstraint<T>::operator~() const {
    return std::make_unique<OutsideSphereConstraint<T>>(radius_, center_);
}

// OutsideSphereConstraint implementation
template<typename T>
OutsideSphereConstraint<T>::OutsideSphereConstraint(T radius, const Vec3<T>& center)
    : radius_(radius), center_(center) {}

template<typename T>
T OutsideSphereConstraint<T>::penalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) return T(0);
    
    T penalty = T(0);
    size_t n_points = points.shape(0);
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        Vec3<T> diff = point - center_;
        T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
        
        if (distance < radius_) {
            penalty += T(1);
        }
    }
    
    return penalty;
}

template<typename T>
xt::xarray<T> OutsideSphereConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = points.shape(0);
    auto grad = xt::xarray<T>::from_shape({n_points, 3});
    
    for (size_t i = 0; i < n_points; ++i) {
        Vec3<T> point{points(i, 0), points(i, 1), points(i, 2)};
        Vec3<T> diff = point - center_;
        T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
        
        if (distance < radius_) {
            // Push point away from center
            if (distance > T(0)) {
                grad(i, 0) = diff[0] / distance;
                grad(i, 1) = diff[1] / distance;
                grad(i, 2) = diff[2] / distance;
            } else {
                // If point is exactly at center, push in random direction
                grad(i, 0) = T(1);
                grad(i, 1) = T(0);
                grad(i, 2) = T(0);
            }
        } else {
            // No gradient for points outside the sphere
            grad(i, 0) = T(0);
            grad(i, 1) = T(0);
            grad(i, 2) = T(0);
        }
    }
    
    return grad;
}

template<typename T>
std::unique_ptr<Constraint<T>> OutsideSphereConstraint<T>::operator~() const {
    return std::make_unique<InsideSphereConstraint<T>>(radius_, center_);
}

// MinDistanceConstraint implementation
template<typename T>
MinDistanceConstraint<T>::MinDistanceConstraint(T min_distance)
    : min_distance_(min_distance) {}

template<typename T>
T MinDistanceConstraint<T>::penalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) return T(0);
    
    T penalty = T(0);
    size_t n_points = points.shape(0);
    
    for (size_t i = 0; i < n_points; ++i) {
        for (size_t j = i + 1; j < n_points; ++j) {
            Vec3<T> point_i{points(i, 0), points(i, 1), points(i, 2)};
            Vec3<T> point_j{points(j, 0), points(j, 1), points(j, 2)};
            
            Vec3<T> diff = point_i - point_j;
            T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
            
            if (distance < min_distance_) {
                penalty += T(1);
            }
        }
    }
    
    return penalty;
}

template<typename T>
xt::xarray<T> MinDistanceConstraint<T>::dpenalty(const xt::xarray<T>& points) const {
    if (points.size() == 0) {
        return xt::xarray<T>::from_shape({0, 3});
    }
    
    size_t n_points = points.shape(0);
    auto grad = xt::xarray<T>::from_shape({n_points, 3});
    
    // Initialize gradients to zero
    for (size_t i = 0; i < n_points; ++i) {
        for (int dim = 0; dim < 3; ++dim) {
            grad(i, dim) = T(0);
        }
    }
    
    for (size_t i = 0; i < n_points; ++i) {
        for (size_t j = i + 1; j < n_points; ++j) {
            Vec3<T> point_i{points(i, 0), points(i, 1), points(i, 2)};
            Vec3<T> point_j{points(j, 0), points(j, 1), points(j, 2)};
            
            Vec3<T> diff = point_i - point_j;
            T distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
            
            if (distance < min_distance_ && distance > T(0)) {
                // Push points apart
                T scale = T(1) / distance;
                for (int dim = 0; dim < 3; ++dim) {
                    grad(i, dim) += diff[dim] * scale;
                    grad(j, dim) -= diff[dim] * scale;
                }
            }
        }
    }
    
    return grad;
}

} // namespace molcpp::pack

