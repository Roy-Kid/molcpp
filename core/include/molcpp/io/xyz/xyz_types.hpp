#pragma once

#include <string>
#include <expected>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <ios>
#include <ctime>
#include "../errors.hpp"

namespace molcpp {

/**
 * @brief Internal data structure for storing atom information during parsing.
 */
struct atom_data {
    std::string symbol;
    double x, y, z;
    double vx = 0.0, vy = 0.0, vz = 0.0;  // Velocities (default to 0)
    bool has_velocity = false;
};

/**
 * @brief Internal structure for storing ExtXYZ metadata.
 */
struct extxyz_metadata {
    std::string raw_comment;
    bool is_extxyz = false;
    std::string lattice_str;
    std::string pbc_str;
    std::string properties_str;
    std::string time_str;
    std::string step_str;
    std::unordered_map<std::string, std::string> extra_meta;
};

/**
 * @brief Internal structure for storing trajectory index information.
 */
struct trajectory_index {
    std::vector<std::streampos> frame_offsets;
    std::time_t file_mtime;
    std::size_t file_size;
    
    bool is_valid(const std::string& file_path) const;
};

} // namespace molcpp