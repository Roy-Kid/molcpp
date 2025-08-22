#ifndef MOLCPP_CORE_FRAME_IMPL_HPP
#define MOLCPP_CORE_FRAME_IMPL_HPP

// Frame implementation with C++20 features

namespace molcpp {

template<typename T>
xt::xarray<T>& Frame::operator()(const std::string& block_key, const std::string& var_key) {
    validate_block_exists(block_key);
    return blocks_[block_key][var_key];
}

template<typename T>
const xt::xarray<T>& Frame::operator()(const std::string& block_key, const std::string& var_key) const {
    validate_block_exists(block_key);
    return blocks_.at(block_key)[var_key];
}

template<typename T>
void Frame::set_variable(const std::string& block_key, const std::string& var_key, const xt::xarray<T>& value) {
    blocks_[block_key].set(var_key, value);
}

template<typename T>
void Frame::set_variable(const std::string& block_key, const std::string& var_key, xt::xarray<T>&& value) {
    blocks_[block_key].set(var_key, std::move(value));
}

template<typename T>
void Frame::set_metadata(const std::string& key, const T& value) {
    metadata_[key] = value;
}

template<typename T>
T Frame::get_metadata(const std::string& key) const {
    auto it = metadata_.find(key);
    if (it == metadata_.end()) {
        throw std::out_of_range(std::format("Metadata key '{}' not found", key));
    }
    
    try {
        return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error(std::format("Type mismatch for metadata key '{}': {}", key, e.what()));
    }
}

template<typename T>
auto Frame::get_variable_span(const std::string& block_key, const std::string& var_key) const 
    -> std::span<const T> {
    validate_block_exists(block_key);
    return blocks_.at(block_key).get_span<T>(var_key);
}

template<typename T>
auto Frame::get_variable_span(const std::string& block_key, const std::string& var_key) 
    -> std::span<T> {
    validate_block_exists(block_key);
    return blocks_[block_key].get_span<T>(var_key);
}

// Inline implementations for basic methods
inline Block& Frame::operator[](const std::string& block_key) {
    return blocks_[block_key];
}

inline const Block& Frame::operator[](const std::string& block_key) const {
    auto it = blocks_.find(block_key);
    if (it == blocks_.end()) {
        throw std::out_of_range(std::format("Block '{}' not found", block_key));
    }
    return it->second;
}

inline Block& Frame::at(const std::string& block_key) {
    auto it = blocks_.find(block_key);
    if (it == blocks_.end()) {
        throw std::out_of_range(std::format("Block '{}' not found", block_key));
    }
    return it->second;
}

inline const Block& Frame::at(const std::string& block_key) const {
    auto it = blocks_.find(block_key);
    if (it == blocks_.end()) {
        throw std::out_of_range(std::format("Block '{}' not found", block_key));
    }
    return it->second;
}

inline void Frame::set_block(const std::string& block_key, const Block& block) {
    blocks_[block_key] = block;
}

inline void Frame::set_block(const std::string& block_key, Block&& block) {
    blocks_[block_key] = std::move(block);
}

inline bool Frame::contains_block(const std::string& block_key) const {
    return blocks_.contains(block_key);
}

inline void Frame::remove_block(const std::string& block_key) {
    blocks_.erase(block_key);
}

inline bool Frame::has_metadata(const std::string& key) const {
    return metadata_.contains(key);
}

inline Frame::size_type Frame::size() const noexcept {
    return blocks_.size();
}

inline Frame::size_type Frame::get_block_count() const noexcept {
    return blocks_.size();
}

inline bool Frame::empty() const noexcept {
    return blocks_.empty();
}

inline Frame::iterator Frame::begin() noexcept {
    return blocks_.begin();
}

inline Frame::iterator Frame::end() noexcept {
    return blocks_.end();
}

inline Frame::const_iterator Frame::begin() const noexcept {
    return blocks_.begin();
}

inline Frame::const_iterator Frame::end() const noexcept {
    return blocks_.end();
}

inline Frame::const_iterator Frame::cbegin() const noexcept {
    return blocks_.cbegin();
}

inline Frame::const_iterator Frame::cend() const noexcept {
    return blocks_.cend();
}

inline Frame::iterator Frame::find_block(const std::string& block_key) {
    return blocks_.find(block_key);
}

inline Frame::const_iterator Frame::find_block(const std::string& block_key) const {
    return blocks_.find(block_key);
}

inline std::vector<std::string> Frame::block_names() const {
    std::vector<std::string> names;
    std::ranges::transform(blocks_, std::back_inserter(names), 
                          [](const auto& pair) { return pair.first; });
    return names;
}

inline std::vector<std::string> Frame::variable_names(const std::string& block_key) const {
    validate_block_exists(block_key);
    const auto& block = blocks_.at(block_key);
    std::vector<std::string> names;
    std::ranges::transform(block, std::back_inserter(names), 
                          [](const auto& pair) { return pair.first; });
    return names;
}

inline Frame::size_type Frame::n_variables(const std::string& block_key) const {
    validate_block_exists(block_key);
    return blocks_.at(block_key).size();
}

inline void Frame::clear() {
    blocks_.clear();
    metadata_.clear();
}

inline void Frame::validate_block_exists(const std::string& block_key) const {
    if (!contains_block(block_key)) {
        throw std::out_of_range(std::format("Block '{}' not found", block_key));
    }
}

inline void Frame::validate_or_throw() const {
    // Validate that all blocks are consistent
    std::ranges::for_each(blocks_, [](const auto& /* pair */) {
        // Add validation logic here if needed
        // For now, we assume blocks are always valid
    });
}

inline bool Frame::validate() const {
    try {
        validate_or_throw();
        return true;
    } catch (...) {
        return false;
    }
}

inline std::string Frame::to_string() const {
    std::string result = std::format("Frame with {} blocks:\n", get_block_count());
    
    for (const auto& [block_name, block] : blocks_) {
        result += std::format("  Block '{}': {} variables\n", block_name, block.size());
    }
    
    if (!metadata_.empty()) {
        result += std::format("  Metadata: {} items\n", metadata_.size());
    }
    
    return result;
}

inline Frame::size_type Frame::n_variables() const {
    size_type total = 0;
    std::ranges::for_each(blocks_, [&total](const auto& pair) {
        total += pair.second.size();
    });
    return total;
}

// Factory function implementations
template<FrameBlock... Blocks>
inline Frame make_frame(const std::string& block_key1, const Blocks&... blocks) {
    Frame frame;
    std::tuple<const Blocks&...> tuple(blocks...);
    
    // Helper to set each block with generated keys
    auto set_block = [&frame, &block_key1]<size_t I>(const auto& block) {
        if constexpr (I == 0) {
            frame.set_block(block_key1, block);
        } else {
            frame.set_block(block_key1 + "_" + std::to_string(I), block);
        }
    };
    
    // Apply to each element in tuple
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (set_block.template operator()<Is>(std::get<Is>(tuple)), ...);
    }(std::index_sequence_for<Blocks...>{});
    
    return frame;
}

// 添加缺失的工厂函数
inline Frame make_frame(const std::map<std::string, Block>& data) {
    Frame frame;
    std::ranges::for_each(data, [&frame](const auto& pair) {
        frame.set_block(pair.first, pair.second);
    });
    return frame;
}

template<typename... Blocks>
inline Frame make_frame_from_blocks(const std::string& block_key1, const Blocks&... blocks) {
    return make_frame(block_key1, blocks...);
}

} // namespace molcpp

#endif // MOLCPP_CORE_FRAME_IMPL_HPP