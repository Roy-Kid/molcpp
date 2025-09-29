#include "molcpp/io/utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace molcpp {

std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss{std::string(str)};
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

std::vector<std::string> split_whitespace(std::string_view str) {
    std::vector<std::string> result;
    std::string current;
    
    for (char c : str) {
        if (c == ' ' || c == '\t') {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

std::string trim(std::string_view str) {
    std::string_view::const_iterator start = str.begin();
    std::string_view::const_iterator end = str.end();
    
    // Trim leading whitespace
    while (start != end && std::isspace(*start)) {
        ++start;
    }
    
    // Trim trailing whitespace
    while (start != end && std::isspace(*(end - 1))) {
        --end;
    }
    
    return std::string(start, end);
}

std::expected<double, io_parse_error> to_double(std::string_view str) {
    try {
        std::string str_copy(str);
        size_t pos;
        double result = std::stod(str_copy, &pos);
        
        if (pos != str_copy.length()) {
            return std::unexpected(io_parse_error(0, "Invalid double value: " + str_copy));
        }
        
        return result;
    } catch (const std::exception&) {
        return std::unexpected(io_parse_error(0, "Invalid double value: " + std::string(str)));
    }
}

std::expected<std::size_t, io_parse_error> to_size_t(std::string_view str) {
    try {
        std::string str_copy(str);
        size_t pos;
        std::size_t result = std::stoull(str_copy, &pos);
        
        if (pos != str_copy.length()) {
            return std::unexpected(io_parse_error(0, "Invalid size_t value: " + str_copy));
        }
        
        return result;
    } catch (const std::exception&) {
        return std::unexpected(io_parse_error(0, "Invalid size_t value: " + std::string(str)));
    }
}

std::expected<bool, io_parse_error> to_bool(std::string_view str) {
    std::string str_copy = trim(str);
    std::transform(str_copy.begin(), str_copy.end(), str_copy.begin(), ::tolower);
    
    if (str_copy == "t" || str_copy == "true" || str_copy == "1") {
        return true;
    } else if (str_copy == "f" || str_copy == "false" || str_copy == "0") {
        return false;
    } else {
        return std::unexpected(io_parse_error(0, "Invalid boolean value: " + std::string(str)));
    }
}

} // namespace molcpp
