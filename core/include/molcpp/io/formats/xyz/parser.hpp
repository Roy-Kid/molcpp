#pragma once

#include <molcpp/core/frame.hpp>
#include <molcpp/types.hpp>
#include <string>
#include <vector>

namespace molcpp::io {

/**
 * @brief Utility functions for parsing XYZ format data
 */
namespace xyz_parser {

/**
 * @brief Parse a complete XYZ frame from lines
 * @param lines Vector of lines containing the complete frame data
 * @return Parsed Frame object
 */
Frame parse_xyz_frame(const std::vector<std::string>& lines);

/**
 * @brief Parse a single atom line from XYZ format
 * @param line String containing atom data "element x y z [additional]"
 * @return Tuple of (element, coordinates, additional_data)
 */
struct AtomData {
    std::string element;
    std::array<float, 3> coordinates;
    std::string additional_data;
};

AtomData parse_atom_line(const std::string& line);

/**
 * @brief Parse the header lines of an XYZ frame
 * @param atom_count_line First line containing number of atoms
 * @param comment_line Second line containing comment
 * @return Tuple of (n_atoms, comment)
 */
std::pair<int, std::string> parse_header(const std::string& atom_count_line, 
                                        const std::string& comment_line);

/**
 * @brief Validate XYZ frame format
 * @param lines Vector of lines to validate
 * @return true if valid XYZ format
 */
bool validate_xyz_frame(const std::vector<std::string>& lines);

} // namespace xyz_parser

} // namespace molcpp::io