#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace molcpp::io::utils {

/**
 * @brief File system utility functions for I/O operations
 */
namespace file {

/**
 * @brief Check if file exists
 */
bool exists(const std::filesystem::path& path);

/**
 * @brief Get file extension
 */
std::string get_extension(const std::filesystem::path& path);

/**
 * @brief Get file stem (filename without extension)
 */
std::string get_stem(const std::filesystem::path& path);

/**
 * @brief Get file size in bytes
 */
std::uintmax_t get_size(const std::filesystem::path& path);

/**
 * @brief Check if path is a regular file
 */
bool is_regular_file(const std::filesystem::path& path);

/**
 * @brief Check if path is a directory
 */
bool is_directory(const std::filesystem::path& path);

/**
 * @brief Create directories recursively
 */
bool create_directories(const std::filesystem::path& path);

/**
 * @brief Get parent directory path
 */
std::filesystem::path parent_path(const std::filesystem::path& path);

/**
 * @brief Join multiple path components
 */
std::filesystem::path join_paths(const std::vector<std::string>& components);

/**
 * @brief Normalize path (resolve .. and . components)
 */
std::filesystem::path normalize_path(const std::filesystem::path& path);

} // namespace file

} // namespace molcpp::io::utils