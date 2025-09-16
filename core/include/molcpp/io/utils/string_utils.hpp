#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace molcpp::io::utils {

/**
 * @brief String utility functions for I/O operations
 */
namespace string {

/**
 * @brief Trim whitespace from both ends of a string
 */
std::string trim(const std::string& str);

/**
 * @brief Convert string to lowercase
 */
std::string to_lower(const std::string& str);

/**
 * @brief Convert string to uppercase
 */
std::string to_upper(const std::string& str);

/**
 * @brief Split string by delimiter
 */
std::vector<std::string> split(const std::string& str, char delimiter);

/**
 * @brief Split string by whitespace
 */
std::vector<std::string> split_whitespace(const std::string& str);

/**
 * @brief Check if string starts with prefix
 */
bool starts_with(const std::string& str, const std::string& prefix);

/**
 * @brief Check if string ends with suffix
 */
bool ends_with(const std::string& str, const std::string& suffix);

/**
 * @brief Replace all occurrences of a substring
 */
std::string replace_all(const std::string& str, const std::string& from, const std::string& to);

} // namespace string

} // namespace molcpp::io::utils