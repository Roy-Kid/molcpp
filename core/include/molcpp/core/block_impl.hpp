#ifndef MOLCPP_CORE_BLOCK_IMPL_HPP
#define MOLCPP_CORE_BLOCK_IMPL_HPP

// Block implementation with C++20 features

namespace molcpp {

// Internal helper for BlockProxy
template<typename T>
xt::xarray<T>& Block::get_typed_ref(const std::string& key) {
    auto it = vars_.find(key);
    if (it == vars_.end()) {
        throw std::out_of_range(std::format("Key '{}' not found", key));
    }
    
    try {
        return std::any_cast<xt::xarray<T>&>(it->second);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error(std::format("Type mismatch for key '{}': {}", key, e.what()));
    }
}

template<typename T>
const xt::xarray<T>& Block::get_typed_ref(const std::string& key) const {
    auto it = vars_.find(key);
    if (it == vars_.end()) {
        throw std::out_of_range(std::format("Key '{}' not found", key));
    }
    
    try {
        return std::any_cast<const xt::xarray<T>&>(it->second);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error(std::format("Type mismatch for key '{}': {}", key, e.what()));
    }
}

inline Block::BlockProxy Block::operator[](const std::string& key) {
    return BlockProxy(*this, key);
}

inline const Block::BlockProxy Block::operator[](const std::string& key) const {
    return BlockProxy(*this, key);
}

inline Block::BlockProxy Block::at(const std::string& key) {
    validate_key_exists(key);
    return BlockProxy(*this, key);
}

inline const Block::BlockProxy Block::at(const std::string& key) const {
    validate_key_exists(key);
    return BlockProxy(*this, key);
}

// Python-like get methods
template<typename T>
xt::xarray<T>& Block::get(const std::string& key) {
    return get_typed_ref<T>(key);
}

template<typename T>
const xt::xarray<T>& Block::get(const std::string& key) const {
    return get_typed_ref<T>(key);
}

inline std::any& Block::get_any(const std::string& key) {
    return vars_[key];
}

inline const std::any& Block::get_any(const std::string& key) const {
    auto it = vars_.find(key);
    if (it == vars_.end()) {
        throw std::out_of_range(std::format("Key '{}' not found", key));
    }
    return it->second;
}



template<XtensorArray T>
void Block::set(const std::string& key, const T& value) {
    vars_[key] = value;
}

template<XtensorArray T>
void Block::set(const std::string& key, T&& value) {
    vars_[key] = std::move(value);
}

template<typename T>
bool Block::has_type(const std::string& key) const {
    auto it = vars_.find(key);
    if (it == vars_.end()) {
        return false;
    }
    
    try {
        std::any_cast<const xt::xarray<T>&>(it->second);
        return true;
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

template<NumericType T>
T Block::get_value_from_any(const std::any& var, size_t index) const {
    // Try different xtensor types
    if (auto* arr_int = std::any_cast<xt::xarray<int>>(&var)) {
        return static_cast<T>((*arr_int)(index));
    } else if (auto* arr_float = std::any_cast<xt::xarray<float>>(&var)) {
        return static_cast<T>((*arr_float)(index));
    } else if (auto* arr_double = std::any_cast<xt::xarray<double>>(&var)) {
        return static_cast<T>((*arr_double)(index));
    } else if (auto* arr_uint = std::any_cast<xt::xarray<unsigned int>>(&var)) {
        return static_cast<T>((*arr_uint)(index));
    } else {
        throw std::runtime_error("Unsupported array type for value extraction");
    }
}

inline Block::size_type Block::nrows() const {
    if (vars_.empty()) {
        return 0;
    }
    
    // Get the size of the first array
    auto first_array = vars_.begin();
    size_type ref_size = 0;
    
    // Use any_cast with different types to find the size
    if (auto* arr_int = std::any_cast<xt::xarray<int>>(&first_array->second)) {
        ref_size = arr_int->size();
    } else if (auto* arr_float = std::any_cast<xt::xarray<float>>(&first_array->second)) {
        ref_size = arr_float->size();
    } else if (auto* arr_double = std::any_cast<xt::xarray<double>>(&first_array->second)) {
        ref_size = arr_double->size();
    } else if (auto* arr_uint = std::any_cast<xt::xarray<unsigned int>>(&first_array->second)) {
        ref_size = arr_uint->size();
    } else {
        throw std::runtime_error("Unsupported array type");
    }
    
    // Validate all arrays have the same size
    for (const auto& [key, value] : vars_) {
        size_type current_size = 0;
        if (auto* arr_int = std::any_cast<xt::xarray<int>>(&value)) {
            current_size = arr_int->size();
        } else if (auto* arr_float = std::any_cast<xt::xarray<float>>(&value)) {
            current_size = arr_float->size();
        } else if (auto* arr_double = std::any_cast<xt::xarray<double>>(&value)) {
            current_size = arr_double->size();
        } else if (auto* arr_uint = std::any_cast<xt::xarray<unsigned int>>(&value)) {
            current_size = arr_uint->size();
        }
        
        if (current_size != ref_size) {
            throw std::runtime_error(std::format("Array size mismatch: expected {}, got {} for key '{}'", 
                                                ref_size, current_size, key));
        }
    }
    
    return ref_size;
}

inline std::string Block::get_type(const std::string& key) const {
    auto it = vars_.find(key);
    if (it == vars_.end()) {
        throw std::out_of_range(std::format("Key '{}' not found", key));
    }
    
    // Try to determine the type by attempting casts
    if (std::any_cast<xt::xarray<int>>(&it->second)) return "int";
    if (std::any_cast<xt::xarray<float>>(&it->second)) return "float";
    if (std::any_cast<xt::xarray<double>>(&it->second)) return "double";
    if (std::any_cast<xt::xarray<unsigned int>>(&it->second)) return "unsigned int";
    if (std::any_cast<xt::xarray<bool>>(&it->second)) return "bool";
    return "unknown";
}

inline std::map<std::string, std::string> Block::get_types() const {
    std::map<std::string, std::string> types;
    std::ranges::for_each(vars_, [&types, this](const auto& pair) {
        types[pair.first] = get_type(pair.first);
    });
    return types;
}

inline Block Block::from_map(const std::map<std::string, std::any>& data) {
    Block block;
    std::ranges::for_each(data, [&block](const auto& pair) {
        block.vars_[pair.first] = pair.second;
    });
    return block;
}

template<typename T>
auto Block::get_span(const std::string& key) const -> std::span<const T> {
    const auto& arr = get_typed_ref<T>(key);
    return std::span<const T>{arr.data(), arr.size()};
}

template<typename T>
auto Block::get_span(const std::string& key) -> std::span<T> {
    auto& arr = get_typed_ref<T>(key);
    return std::span<T>{arr.data(), arr.size()};
}

// Inline implementations for basic methods
inline Block::size_type Block::size() const noexcept {
    return vars_.size();
}

inline bool Block::empty() const noexcept {
    return vars_.empty();
}

inline void Block::erase(const std::string& key) {
    vars_.erase(key);
}

inline void Block::clear() {
    vars_.clear();
}

inline bool Block::contains(const std::string& key) const {
    return vars_.contains(key);
}

inline Block::iterator Block::find(const std::string& key) {
    return vars_.find(key);
}

inline Block::const_iterator Block::find(const std::string& key) const {
    return vars_.find(key);
}

inline Block::iterator Block::begin() noexcept {
    return vars_.begin();
}

inline Block::iterator Block::end() noexcept {
    return vars_.end();
}

inline Block::const_iterator Block::begin() const noexcept {
    return vars_.begin();
}

inline Block::const_iterator Block::end() const noexcept {
    return vars_.end();
}

inline Block::const_iterator Block::cbegin() const noexcept {
    return vars_.cbegin();
}

inline Block::const_iterator Block::cend() const noexcept {
    return vars_.cend();
}

inline std::map<std::string, std::any> Block::to_map() const {
    return vars_;
}

inline void Block::validate_key_exists(const std::string& key) const {
    if (!contains(key)) {
        throw std::out_of_range(std::format("Key '{}' not found", key));
    }
}

inline bool Block::is_valid_array_type(const std::any& value) noexcept {
    return value.has_value();
}

// Factory function implementations
template<typename T>
Block make_block(const std::map<std::string, xt::xarray<T>>& data) {
    Block block;
    std::ranges::for_each(data, [&block](const auto& pair) {
        block.set(pair.first, pair.second);
    });
    return block;
}

template<typename... Ts>
Block make_block_from_arrays(const std::string& key1, const xt::xarray<Ts>&... arrays) {
    Block block;
    std::tuple<const xt::xarray<Ts>&...> tuple(arrays...);
    
    // Helper to set each array with generated keys
    auto set_array = [&block, &key1]<size_t I>(const auto& arr) {
        if constexpr (I == 0) {
            block.set(key1, arr);
        } else {
            block.set(key1 + "_" + std::to_string(I), arr);
        }
    };
    
    // Apply to each element in tuple
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (set_array.template operator()<Is>(std::get<Is>(tuple)), ...);
    }(std::index_sequence_for<Ts...>{});
    
    return block;
}

} // namespace molcpp

#endif // MOLCPP_CORE_BLOCK_IMPL_HPP