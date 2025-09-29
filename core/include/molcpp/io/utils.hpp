#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <expected>
#include <cstddef>
#include "errors.hpp"

namespace molcpp {

/**
 * @brief Split string by delimiter.
 * @param str The string to split
 * @param delimiter The delimiter character
 * @return Vector of split strings
 */
std::vector<std::string> split(std::string_view str, char delimiter);

/**
 * @brief Split string by whitespace (spaces and tabs).
 * @param str The string to split
 * @return Vector of split strings
 */
std::vector<std::string> split_whitespace(std::string_view str);

/**
 * @brief Trim whitespace from string.
 * @param str The string to trim
 * @return Trimmed string
 */
std::string trim(std::string_view str);

/**
 * @brief Convert string to double.
 * @param str The string to convert
 * @return Expected double value or error
 */
std::expected<double, io_parse_error> to_double(std::string_view str);

/**
 * @brief Convert string to size_t.
 * @param str The string to convert
 * @return Expected size_t value or error
 */
std::expected<std::size_t, io_parse_error> to_size_t(std::string_view str);

/**
 * @brief Convert string to boolean.
 * @param str The string to convert
 * @return Expected boolean value or error
 */
std::expected<bool, io_parse_error> to_bool(std::string_view str);

} // namespace molcpp
