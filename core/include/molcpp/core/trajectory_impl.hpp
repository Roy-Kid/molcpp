#pragma once

#include "trajectory.hpp"
#include <algorithm>
#include <stdexcept>

namespace molcpp::core {

// =============================================================================
// Trajectory Implementation
// =============================================================================

template<FrameProvider T>
Trajectory::Trajectory(T& provider) 
    : provider_(std::make_unique<FrameProviderWrapper<T>>(provider)) {}

frame_type Trajectory::read_frame(size_t index) const {
    if (!provider_) {
        throw std::runtime_error("No frame provider available");
    }
    
    // Check if frame is cached
    if (caching_enabled_ && index < frame_cache_.size() && frame_cache_[index].has_value()) {
        return frame_cache_[index].value();
    }
    
    // Read frame from provider
    auto frame = provider_->read_frame(index);
    
    // Cache if enabled
    if (caching_enabled_) {
        if (index >= frame_cache_.size()) {
            frame_cache_.resize(index + 1);
        }
        frame_cache_[index] = frame;
    }
    
    return frame;
}

std::vector<frame_type> Trajectory::read_frames(const std::vector<size_t>& indices) const {
    std::vector<frame_type> frames;
    frames.reserve(indices.size());
    
    for (size_t index : indices) {
        frames.push_back(read_frame(index));
    }
    
    return frames;
}

std::vector<frame_type> Trajectory::read_range(size_t start, size_t stop, size_t step) const {
    std::vector<frame_type> frames;
    
    for (size_t i = start; i < stop; i += step) {
        if (i < n_frames()) {
            frames.push_back(read_frame(i));
        }
    }
    
    return frames;
}

std::vector<frame_type> Trajectory::read_all() const {
    std::vector<frame_type> frames;
    frames.reserve(n_frames());
    
    for (size_t i = 0; i < n_frames(); ++i) {
        frames.push_back(read_frame(i));
    }
    
    return frames;
}

size_t Trajectory::n_frames() const {
    return provider_ ? provider_->n_frames() : 0;
}

bool Trajectory::empty() const {
    return provider_ ? provider_->empty() : true;
}

void Trajectory::enable_caching(bool enable) {
    caching_enabled_ = enable;
    if (!enable) {
        clear_cache();
    }
}

void Trajectory::clear_cache() {
    frame_cache_.clear();
}

bool Trajectory::is_caching_enabled() const {
    return caching_enabled_;
}

} // namespace molcpp::core
