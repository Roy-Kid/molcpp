#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "xyz_types.hpp"
#include "molcpp/io/utils.hpp"
#include <xtensor/containers/xarray.hpp>

// Forward declaration
namespace molcpp {
    class Frame;
}

namespace molcpp {

/**
 * @brief Check if a comment line looks like ExtXYZ format.
 * @param line The comment line to check
 * @return true if the line contains key=value pairs
 */
bool looks_like_extxyz_comment(std::string_view line);

/**
 * @brief Parse ExtXYZ key=value pairs from a comment line.
 * @param line The comment line to parse
 * @return Map of key-value pairs
 */
std::unordered_map<std::string, std::string> parse_extxyz_kv(std::string_view line);

/**
 * @brief Parse lattice matrix from ExtXYZ format.
 * @param lattice_str The lattice string (9 space-separated values)
 * @return 3x3 matrix as vector of vectors
 */
std::expected<xt::xarray<double>, xyz_parse_error> parse_lattice_matrix(std::string_view lattice_str);

/**
 * @brief Parse PBC flags from ExtXYZ format.
 * @param pbc_str The PBC string (3 space-separated T/F values)
 * @return Vector of 3 boolean values
 */
std::expected<xt::xarray<bool>, xyz_parse_error> parse_pbc_flags(std::string_view pbc_str);

/**
 * @brief Parse time value from ExtXYZ format.
 * @param time_str The time string
 * @return Parsed time value
 */
std::expected<double, xyz_parse_error> parse_time(std::string_view time_str);

/**
 * @brief Parse step value from ExtXYZ format.
 * @param step_str The step string
 * @return Parsed step value
 */
std::expected<std::size_t, xyz_parse_error> parse_step(std::string_view step_str);

/**
 * @brief Check if Properties string indicates velocities are present.
 * @param properties_str The Properties string
 * @return true if velocities are present
 */
bool has_velocities(std::string_view properties_str);

/**
 * @brief Validate Properties string format.
 * @param properties_str The Properties string to validate
 * @return true if format is valid
 */
bool validate_properties_format(std::string_view properties_str);

/**
 * @brief Build ExtXYZ comment line from Frame data.
 * @param frame The frame to build comment from
 * @return ExtXYZ comment string
 */
std::string build_extxyz_comment(const molcpp::Frame& frame);

/**
 * @brief Convert element symbol to atomic number.
 * @param symbol Element symbol (e.g., "H", "He", "Li")
 * @return Atomic number (1 for H, 2 for He, etc.)
 */
int symbol_to_atomic_number(const std::string& symbol);

/**
 * @brief Convert atomic number to element symbol.
 * @param atomic_number Atomic number (1 for H, 2 for He, etc.)
 * @return Element symbol (e.g., "H", "He", "Li")
 */
std::string atomic_number_to_symbol(int atomic_number);

} // namespace molcpp