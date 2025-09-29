#pragma once

#include <string>
#include <cstddef>

namespace molcpp {

/**
 * @brief Base error type for IO parsing failures.
 */
struct io_parse_error {
    std::size_t line;      ///< Line number where error occurred
    std::string message;   ///< Error description
    
    io_parse_error(std::size_t line_num, std::string msg) 
        : line(line_num), message(std::move(msg)) {}
};

/**
 * @brief Error type for XYZ parsing failures.
 */
struct xyz_parse_error : public io_parse_error {
    xyz_parse_error(std::size_t line_num, std::string msg) 
        : io_parse_error(line_num, std::move(msg)) {}
};

} // namespace molcpp
