#pragma once

#include "frame.hpp"
#include <memory>
#include <vector>
#include <concepts>

namespace molcpp::core {

// Concept for frame providers
template<typename T>
concept FrameProvider = requires(T t) {
    { t.read_frame(size_t{}) } -> std::same_as<Frame>;
    { t.n_frames() } -> std::same_as<size_t>;
    { t.empty() } -> std::same_as<bool>;
};

/**
 * @brief Trajectory class that wraps a FrameProvider for caching and memory management.
 * 
 * This class acts as a smart wrapper around any object that provides frames,
 * allowing for lazy loading and optional caching of frames in memory.
 */
class Trajectory {
public:
    using frame_type = Frame;
    
    // Constructor from FrameProvider
    template<FrameProvider T>
    explicit Trajectory(T& provider) : provider_(std::make_unique<FrameProviderWrapper<T>>(provider)) {}
    
    // Copy constructor
    Trajectory(const Trajectory& other) = default;
    
    // Move constructor
    Trajectory(Trajectory&& other) noexcept = default;
    
    // Assignment operators
    Trajectory& operator=(const Trajectory& other) = default;
    Trajectory& operator=(Trajectory&& other) noexcept = default;
    
    // Destructor
    ~Trajectory() = default;
    
    // Frame access methods
    frame_type read_frame(size_t index) const;
    std::vector<frame_type> read_frames(const std::vector<size_t>& indices) const;
    std::vector<frame_type> read_range(size_t start, size_t stop, size_t step = 1) const;
    std::vector<frame_type> read_all() const;
    
    // Properties
    size_t n_frames() const;
    bool empty() const;
    
    // Caching control
    void enable_caching(bool enable = true);
    void clear_cache();
    bool is_caching_enabled() const;

private:
    // Interface for frame providers
    class FrameProviderInterface {
    public:
        virtual ~FrameProviderInterface() = default;
        virtual frame_type read_frame(size_t index) const = 0;
        virtual size_t n_frames() const = 0;
        virtual bool empty() const = 0;
    };
    
    // Template wrapper for concrete FrameProvider types
    template<FrameProvider T>
    class FrameProviderWrapper : public FrameProviderInterface {
    public:
        explicit FrameProviderWrapper(T& provider) : provider_(provider) {}
        
        frame_type read_frame(size_t index) const override {
            return provider_.read_frame(index);
        }
        
        size_t n_frames() const override {
            return provider_.n_frames();
        }
        
        bool empty() const override {
            return provider_.empty();
        }
        
    private:
        T& provider_;
    };
    
    std::unique_ptr<FrameProviderInterface> provider_;
    mutable std::vector<std::optional<frame_type>> frame_cache_;
    bool caching_enabled_ = false;
};

} // namespace molcpp::core
