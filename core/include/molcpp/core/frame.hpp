#ifndef MOLCPP_CORE_FRAME_HPP
#define MOLCPP_CORE_FRAME_HPP

#include "block.hpp"
#include <map>
#include <string>
#include <any>
#include <concepts>
#include <ranges>
#include <span>
#include <format>
#include "paramdict.hpp"

namespace molcpp {

// C++20 concepts for Frame
template<typename T>
concept FrameBlock = std::same_as<T, Block>;

template<typename T>
concept FrameMetadata = std::same_as<T, std::any>;

// 导入Block的概念
template<typename T>
concept XtensorNumericArray = molcpp::XtensorArray<T>;

/**
 * @brief Hierarchical container holding multiple Block instances and metadata.
 * 
 * C++20 enhanced Frame class that stores named Block instances and metadata.
 * Provides a clean interface for accessing variables across different blocks.
 */
class Frame {
public:
    using size_type = size_t;
    using block_type = Block;
    using metadata_type = std::any;
    using iterator = typename std::map<std::string, Block>::iterator;
    using const_iterator = typename std::map<std::string, Block>::const_iterator;

    // Constructors
    Frame() = default;
    Frame(const Frame& other) = default;
    Frame(Frame&& other) noexcept = default;
    
    // Assignment operators
    Frame& operator=(const Frame& other) = default;
    Frame& operator=(Frame&& other) noexcept = default;

    // Destructor
    ~Frame() = default;

    // Block access
    Block& operator[](const std::string& block_key);
    const Block& operator[](const std::string& block_key) const;
    
    // Safe block access (throws on missing key)
    Block& at(const std::string& block_key);
    const Block& at(const std::string& block_key) const;
    
    // Variable access across blocks
    template<typename T>
    xt::xarray<T>& operator()(const std::string& block_key, const std::string& var_key);
    
    template<typename T>
    const xt::xarray<T>& operator()(const std::string& block_key, const std::string& var_key) const;
    
    // Block management
    void set_block(const std::string& block_key, const Block& block);
    void set_block(const std::string& block_key, Block&& block);
    bool contains_block(const std::string& block_key) const;
    void remove_block(const std::string& block_key);
    
    // Variable management across blocks
    template<typename T>
    void set_variable(const std::string& block_key, const std::string& var_key, const xt::xarray<T>& value);
    
    template<typename T>
    void set_variable(const std::string& block_key, const std::string& var_key, xt::xarray<T>&& value);
    
    // Metadata management
    template<typename T>
    void set_metadata(const std::string& key, const T& value);
    
    template<typename T>
    T get_metadata(const std::string& key) const;
    
    bool has_metadata(const std::string& key) const;
    ParamDict metadata_dict() const { return ParamDict(&metadata_); }
    
    // 兼容性方法
    bool contains_metadata(const std::string& key) const { return has_metadata(key); }
    
    // 验证方法
    bool validate() const;
    
    // 字符串表示
    std::string to_string() const;
    
    // 统计方法
    size_type n_blocks() const { return get_block_count(); }
    size_type n_variables() const;
    
    // Size and capacity
    size_type size() const noexcept;
    bool empty() const noexcept;
    size_type get_block_count() const noexcept;
    
    // Iteration
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    
    // Range-based access
    auto blocks() const { 
        return std::views::transform(blocks_, [](const auto& pair) { return pair.first; });
    }
    auto block_values() const { 
        return std::views::transform(blocks_, [](const auto& pair) { return pair.second; }); 
    }
    auto block_items() const { 
        return std::views::transform(blocks_, [](const auto& pair) { return std::make_pair(pair.first, pair.second); });
    }
    
    // Lookup
    iterator find_block(const std::string& block_key);
    const_iterator find_block(const std::string& block_key) const;
    
    // Block and variable information
    std::vector<std::string> block_names() const;
    std::vector<std::string> variable_names(const std::string& block_key) const;
    size_type n_variables(const std::string& block_key) const;
    
    // Utility methods
    void clear();
    
    // C++20 span access for variables
    template<typename T>
    auto get_variable_span(const std::string& block_key, const std::string& var_key) const 
        -> std::span<const T>;
    
    template<typename T>
    auto get_variable_span(const std::string& block_key, const std::string& var_key) 
        -> std::span<T>;
    
    // Non-constexpr methods
    bool is_empty() const noexcept { return blocks_.empty(); }

private:
    std::map<std::string, Block> blocks_;
    std::map<std::string, metadata_type> metadata_;
    
    // Helper methods
    void validate_block_exists(const std::string& block_key) const;
    void validate_or_throw() const;
};

// C++20 concept-based factory functions
template<FrameBlock... Blocks>
Frame make_frame(const std::string& block_key1, const Blocks&... blocks);

} // namespace molcpp

// Include implementation
#include "frame_impl.hpp"

#endif // MOLCPP_CORE_FRAME_HPP