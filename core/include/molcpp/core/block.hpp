#ifndef MOLCPP_CORE_BLOCK_HPP
#define MOLCPP_CORE_BLOCK_HPP

#include <xtensor/containers/xarray.hpp>
#include <map>
#include <string>
#include <any>
#include <concepts>
#include <ranges>
#include <span>
#include <format>

namespace molcpp {

// C++20 concepts for xtensor arrays
template<typename T>
concept NumericType = std::is_arithmetic_v<T>;

// 概念：用于set方法的xtensor类型
template<typename T>
concept XtensorArray = requires(T t) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
};

/**
 * @brief Heterogeneous Block that can store different types of xtensor arrays.
 * 
 * C++20 enhanced version with concepts, ranges, and modern features.
 * Inspired by xframe design, this class provides a clean interface for storing
 * mixed-type arrays while maintaining type safety and performance.
 */
class Block {
public:
    using size_type = size_t;
    using iterator = typename std::map<std::string, std::any>::iterator;
    using const_iterator = typename std::map<std::string, std::any>::const_iterator;

    // Constructors
    Block() = default;
    Block(const Block& other) = default;
    Block(Block&& other) noexcept = default;
    
    // Assignment operators
    Block& operator=(const Block& other) = default;
    Block& operator=(Block&& other) noexcept = default;

    // Destructor
    ~Block() = default;

    // Core mapping API - smart proxy with explicit type conversion required
    // IMPORTANT: When using operator[], you MUST specify the type explicitly
    // WRONG: auto data = block["key"];  // This gives BlockProxy, not xt::xarray
    // CORRECT: xt::xarray<double> data = block["key"];  // Explicit type declaration
    class BlockProxy;
    BlockProxy operator[](const std::string& key);
    const BlockProxy operator[](const std::string& key) const;
    
    // Safe access that throws on missing key
    BlockProxy at(const std::string& key);
    const BlockProxy at(const std::string& key) const;
    
    // Python-like methods
    template<typename T>
    xt::xarray<T>& get(const std::string& key);
    
    template<typename T>
    const xt::xarray<T>& get(const std::string& key) const;
    
    // Raw access for internal use only
    std::any& get_any(const std::string& key);
    const std::any& get_any(const std::string& key) const;
    
    // Assignment methods with concepts
    template<XtensorArray T>
    void set(const std::string& key, const T& value);
    
    template<XtensorArray T>
    void set(const std::string& key, T&& value);
    
    // Type checking
    template<typename T>
    bool has_type(const std::string& key) const;
    
    // Type information
    std::string get_type(const std::string& key) const;
    std::map<std::string, std::string> get_types() const;
    
    // Removal
    void erase(const std::string& key);
    void clear();
    
    // Size and capacity
    size_type size() const noexcept;
    bool empty() const noexcept;
    size_type nrows() const;
    
    // Iteration with ranges
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    
    // Range-based access
    auto keys() const { 
        return std::views::transform(vars_, [](const auto& pair) { return pair.first; });
    }
    auto values() const { 
        return std::views::transform(vars_, [](const auto& pair) { return pair.second; }); 
    }
    auto items() const { 
        return std::views::transform(vars_, [](const auto& pair) { return std::make_pair(pair.first, pair.second); });
    }
    
    // Lookup
    bool contains(const std::string& key) const;
    iterator find(const std::string& key);
    const_iterator find(const std::string& key) const;
    
    // Utility methods
    std::map<std::string, std::any> to_map() const;
    static Block from_map(const std::map<std::string, std::any>& data);
    
    // C++20 enhanced methods
    template<typename T>
    auto get_span(const std::string& key) const -> std::span<const T>;
    
    template<typename T>
    auto get_span(const std::string& key) -> std::span<T>;
    
    // Non-constexpr methods (std::map methods are not constexpr)
    bool is_empty() const noexcept { return vars_.empty(); }
    size_type get_size() const noexcept { return vars_.size(); }

public:
    // Smart proxy class for explicit type conversion
    // IMPORTANT: This class requires explicit type conversion to get xt::xarray types
    // Usage patterns:
    // 1. xt::xarray<double> data = block["key"];  // Copy
    // 2. xt::xarray<double>& ref = block["key"];  // Reference (direct assignment)
    // 3. xt::xarray<double>& ref = block.get<double>("key");  // Direct get method
    class BlockProxy {
    private:
        const Block* block_ptr_;
        Block* block_ptr_mutable_;
        const std::string& key_;
        bool is_const_;
        
    public:
        BlockProxy(Block& block, const std::string& key) 
            : block_ptr_(nullptr), block_ptr_mutable_(&block), key_(key), is_const_(false) {}
        
        BlockProxy(const Block& block, const std::string& key) 
            : block_ptr_(&block), block_ptr_mutable_(nullptr), key_(key), is_const_(true) {}
        
        // Auto deduction - returns the actual xtensor type
        template<typename T>
        operator xt::xarray<T>&() {
            if (is_const_) {
                throw std::runtime_error("Cannot get mutable reference from const Block");
            }
            return block_ptr_mutable_->get_typed_ref<T>(key_);
        }
        
        template<typename T>
        operator const xt::xarray<T>&() const {
            if (is_const_) {
                return block_ptr_->get_typed_ref<T>(key_);
            } else {
                return block_ptr_mutable_->get_typed_ref<T>(key_);
            }
        }
        
        // Also support conversion to value types (for const blocks)
        template<typename T>
        operator xt::xarray<T>() const {
            if (is_const_) {
                return block_ptr_->get_typed_ref<T>(key_);
            } else {
                return block_ptr_mutable_->get_typed_ref<T>(key_);
            }
        }
        
        // Assignment operator
        template<typename T>
        xt::xarray<T>& operator=(const xt::xarray<T>& value) {
            if (is_const_) {
                throw std::runtime_error("Cannot assign to const Block");
            }
            block_ptr_mutable_->set(key_, value);
            return block_ptr_mutable_->get_typed_ref<T>(key_);
        }
        
        // Move assignment operator
        template<typename T>
        xt::xarray<T>& operator=(xt::xarray<T>&& value) {
            if (is_const_) {
                throw std::runtime_error("Cannot assign to const Block");
            }
            block_ptr_mutable_->set(key_, std::move(value));
            return block_ptr_mutable_->get_typed_ref<T>(key_);
        }
    };

private:
    std::map<std::string, std::any> vars_;
    
    // Internal helper for BlockProxy
    template<typename T>
    xt::xarray<T>& get_typed_ref(const std::string& key);
    
    template<typename T>
    const xt::xarray<T>& get_typed_ref(const std::string& key) const;
    
    // Helper methods
    void validate_key_exists(const std::string& key) const;
    void validate_arrays_same_length(const std::string& key) const;
    
    // Helper to get value from any with concepts
    template<NumericType T>
    T get_value_from_any(const std::any& var, size_t index) const;
    
    // C++20 helper
    static bool is_valid_array_type(const std::any& value) noexcept;
};

// C++20 concept-based factory functions
template<typename T>
Block make_block(const std::map<std::string, xt::xarray<T>>& data);

template<typename... Ts>
Block make_block_from_arrays(const std::string& key1, const xt::xarray<Ts>&... arrays);

} // namespace molcpp

// Include implementation
#include "block_impl.hpp"

#endif // MOLCPP_CORE_BLOCK_HPP
