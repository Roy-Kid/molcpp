#include "molcpp/io/xyz/xyz_utils.hpp"
#include "molcpp/core/frame.hpp"
#include <xtensor/containers/xarray.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <unordered_map>

namespace molcpp {

bool looks_like_extxyz_comment(std::string_view line) {
    // Check if line contains key=value pairs
    return line.find('=') != std::string_view::npos;
}

std::unordered_map<std::string, std::string> parse_extxyz_kv(std::string_view line) {
    std::unordered_map<std::string, std::string> result;
    
    std::string line_str(line);
    
    // More robust parsing that handles quoted values
    std::size_t pos = 0;
    while (pos < line_str.length()) {
        // Skip whitespace
        while (pos < line_str.length() && std::isspace(line_str[pos])) {
            pos++;
        }
        if (pos >= line_str.length()) break;
        
        // Find key
        std::size_t key_start = pos;
        while (pos < line_str.length() && line_str[pos] != '=') {
            pos++;
        }
        if (pos >= line_str.length()) break;
        
        std::string key = line_str.substr(key_start, pos - key_start);
        pos++; // skip '='
        
        // Find value
        std::size_t value_start = pos;
        if (pos < line_str.length() && line_str[pos] == '"') {
            // Quoted value
            pos++; // skip opening quote
            while (pos < line_str.length() && line_str[pos] != '"') {
                pos++;
            }
            if (pos < line_str.length()) {
                pos++; // skip closing quote
            }
        } else {
            // Unquoted value - find next space or end
            while (pos < line_str.length() && !std::isspace(line_str[pos])) {
                pos++;
            }
        }
        
        std::string value = line_str.substr(value_start, pos - value_start);
        
        // Remove quotes if present
        if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        
        result[key] = value;
    }
    
    return result;
}

std::expected<xt::xarray<double>, xyz_parse_error> parse_lattice_matrix(std::string_view lattice_str) {
    std::vector<std::string> tokens = split(lattice_str, ' ');
    
    if (tokens.size() != 9) {
        return std::unexpected(xyz_parse_error(0, "Lattice string must contain 9 space-separated doubles."));
    }
    
    xt::xarray<double> lattice = xt::xarray<double>::from_shape({3, 3});
    for (std::size_t i = 0; i < 9; ++i) {
        std::expected<double, io_parse_error> val_result = to_double(tokens[i]);
        if (!val_result) {
            return std::unexpected(xyz_parse_error(0, "Invalid double in Lattice string: " + val_result.error().message));
        }
        lattice.flat(i) = val_result.value();
    }
    
    return lattice;
}

std::expected<xt::xarray<bool>, xyz_parse_error> parse_pbc_flags(std::string_view pbc_str) {
    std::vector<std::string> tokens = split(pbc_str, ' ');
    
    if (tokens.size() != 3) {
        return std::unexpected(xyz_parse_error(0, "pbc string must contain 3 space-separated boolean values."));
    }
    
    xt::xarray<bool> pbc = xt::xarray<bool>::from_shape({3});
    for (std::size_t i = 0; i < 3; ++i) {
        std::expected<bool, io_parse_error> val_result = to_bool(tokens[i]);
        if (!val_result) {
            return std::unexpected(xyz_parse_error(0, "Invalid boolean in pbc string: " + val_result.error().message));
        }
        pbc.flat(i) = val_result.value();
    }
    
    return pbc;
}

std::expected<double, xyz_parse_error> parse_time(std::string_view time_str) {
    std::expected<double, io_parse_error> result = to_double(time_str);
    if (result) {
        return result.value();
    } else {
        return std::unexpected(xyz_parse_error(0, result.error().message));
    }
}

std::expected<std::size_t, xyz_parse_error> parse_step(std::string_view step_str) {
    std::expected<std::size_t, io_parse_error> result = to_size_t(step_str);
    if (result) {
        return result.value();
    } else {
        return std::unexpected(xyz_parse_error(0, result.error().message));
    }
}

bool has_velocities(std::string_view properties_str) {
    // Split by colons and check if any property name starts with "vel"
    std::vector<std::string> specs = split(properties_str, ':');
    
    for (std::size_t i = 0; i < specs.size(); i += 3) {
        if (i < specs.size()) {
            const std::string& name = specs[i];
            if (name.length() >= 3 && name.substr(0, 3) == "vel") {
                return true;
            }
        }
    }
    
    return false;
}

bool validate_properties_format(std::string_view properties_str) {
    if (properties_str.empty()) {
        return false;
    }
    
    // Split by colons to get property specifications
    std::vector<std::string> specs = split(properties_str, ':');
    
    // Must have multiple of 3 parts (name:type:repeat triplets)
    if (specs.size() % 3 != 0) {
        return false;
    }
    
    // Validate each property specification
    for (std::size_t i = 0; i < specs.size(); i += 3) {
        const std::string& name = specs[i];
        const std::string& type = specs[i + 1];
        const std::string& repeat_str = specs[i + 2];
        
        // Name cannot be empty
        if (name.empty()) {
            return false;
        }
        
        // Validate type
        if (type != "S" && type != "I" && type != "R" && type != "L") {
            return false;
        }
        
        // Validate repeat count
        if (repeat_str.empty()) {
            return false;
        }
        
        // Check if repeat is a valid number
        try {
            int repeat = std::stoi(repeat_str);
            if (repeat <= 0) {
                return false;
            }
        } catch (const std::exception&) {
            return false;
        }
    }
    
    // Check that properties are in correct order: species first, then pos, then others
    if (specs.size() >= 3) {
        const std::string& first_name = specs[0];
        if (first_name != "species") {
            return false;
        }
    }
    
    if (specs.size() >= 6) {
        const std::string& second_name = specs[3];
        if (second_name != "pos") {
            return false;
        }
    }
    
    return true;
}

std::string build_extxyz_comment(const molcpp::Frame& frame) {
    std::ostringstream comment;
    
    // Check if we have lattice information
    if (frame.has_metadata("lattice")) {
        const xt::xarray<double>& lattice = frame.get_metadata<xt::xarray<double>>("lattice");
        comment << "Lattice=\"";
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t j = 0; j < 3; ++j) {
                if (i > 0 || j > 0) comment << " ";
                comment << lattice(i, j);
            }
        }
        comment << "\"";
    }
    
    // Check if we have PBC information
    if (frame.has_metadata("pbc")) {
        const xt::xarray<bool>& pbc = frame.get_metadata<xt::xarray<bool>>("pbc");
        if (comment.tellp() > 0) comment << " ";
        comment << "pbc=\"";
        for (std::size_t i = 0; i < 3; ++i) {
            if (i > 0) comment << " ";
            comment << (pbc(i) ? "T" : "F");
        }
        comment << "\"";
    }
    
    // Check if we have Properties information (for velocities)
    if (frame.contains_block("atoms")) {
        const Block& atoms = frame["atoms"];
        if (atoms.contains("velocities")) {
            if (comment.tellp() > 0) comment << " ";
            comment << "Properties=species:S:1:pos:R:3:vel:R:3";
        } else {
            if (comment.tellp() > 0) comment << " ";
            comment << "Properties=species:S:1:pos:R:3";
        }
    }
    
    // Check if we have time information
    if (frame.has_metadata("time")) {
        const double& time = frame.get_metadata<double>("time");
        if (comment.tellp() > 0) comment << " ";
        comment << "Time=" << time;
    }
    
    // Check if we have step information
    if (frame.has_metadata("step")) {
        const std::size_t& step = frame.get_metadata<std::size_t>("step");
        if (comment.tellp() > 0) comment << " ";
        comment << "Step=" << step;
    }
    
    // Add other metadata
    if (frame.has_metadata("comment")) {
        const std::string& raw_comment = frame.get_metadata<std::string>("comment");
        if (raw_comment.find('=') == std::string::npos) {
            // It's a simple comment, not ExtXYZ format
            if (comment.tellp() > 0) comment << " ";
            comment << raw_comment;
        }
    }
    
    std::string result = comment.str();
    return result.empty() ? "Generated by molcpp XYZ writer" : result;
}

int symbol_to_atomic_number(const std::string& symbol) {
    // Simple lookup table for common elements
    static const std::unordered_map<std::string, int> element_map = {
        {"H", 1}, {"He", 2}, {"Li", 3}, {"Be", 4}, {"B", 5}, {"C", 6}, {"N", 7}, {"O", 8}, 
        {"F", 9}, {"Ne", 10}, {"Na", 11}, {"Mg", 12}, {"Al", 13}, {"Si", 14}, {"P", 15}, 
        {"S", 16}, {"Cl", 17}, {"Ar", 18}, {"K", 19}, {"Ca", 20}, {"Sc", 21}, {"Ti", 22}, 
        {"V", 23}, {"Cr", 24}, {"Mn", 25}, {"Fe", 26}, {"Co", 27}, {"Ni", 28}, {"Cu", 29}, 
        {"Zn", 30}, {"Ga", 31}, {"Ge", 32}, {"As", 33}, {"Se", 34}, {"Br", 35}, {"Kr", 36}
    };
    
    auto it = element_map.find(symbol);
    return it != element_map.end() ? it->second : 0; // Return 0 for unknown elements
}

std::string atomic_number_to_symbol(int atomic_number) {
    // Simple lookup table for common elements
    static const std::unordered_map<int, std::string> symbol_map = {
        {1, "H"}, {2, "He"}, {3, "Li"}, {4, "Be"}, {5, "B"}, {6, "C"}, {7, "N"}, {8, "O"}, 
        {9, "F"}, {10, "Ne"}, {11, "Na"}, {12, "Mg"}, {13, "Al"}, {14, "Si"}, {15, "P"}, 
        {16, "S"}, {17, "Cl"}, {18, "Ar"}, {19, "K"}, {20, "Ca"}, {21, "Sc"}, {22, "Ti"}, 
        {23, "V"}, {24, "Cr"}, {25, "Mn"}, {26, "Fe"}, {27, "Co"}, {28, "Ni"}, {29, "Cu"}, 
        {30, "Zn"}, {31, "Ga"}, {32, "Ge"}, {33, "As"}, {34, "Se"}, {35, "Br"}, {36, "Kr"}
    };
    
    auto it = symbol_map.find(atomic_number);
    return it != symbol_map.end() ? it->second : "X"; // Return "X" for unknown elements
}

} // namespace molcpp