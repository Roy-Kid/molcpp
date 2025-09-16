#include <molcpp/io/utils/file_utils.hpp>
#include <system_error>

namespace molcpp::io::utils::file {

bool exists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

std::string get_extension(const std::filesystem::path& path) {
    return path.extension().string();
}

std::string get_stem(const std::filesystem::path& path) {
    return path.stem().string();
}

std::uintmax_t get_size(const std::filesystem::path& path) {
    std::error_code ec;
    auto size = std::filesystem::file_size(path, ec);
    if (ec) {
        throw std::runtime_error("Failed to get file size: " + path.string());
    }
    return size;
}

bool is_regular_file(const std::filesystem::path& path) {
    return std::filesystem::is_regular_file(path);
}

bool is_directory(const std::filesystem::path& path) {
    return std::filesystem::is_directory(path);
}

bool create_directories(const std::filesystem::path& path) {
    std::error_code ec;
    bool result = std::filesystem::create_directories(path, ec);
    return !ec && result;
}

std::filesystem::path parent_path(const std::filesystem::path& path) {
    return path.parent_path();
}

std::filesystem::path join_paths(const std::vector<std::string>& components) {
    if (components.empty()) {
        return {};
    }
    
    std::filesystem::path result = components[0];
    for (size_t i = 1; i < components.size(); ++i) {
        result /= components[i];
    }
    return result;
}

std::filesystem::path normalize_path(const std::filesystem::path& path) {
    return std::filesystem::canonical(path);
}

} // namespace molcpp::io::utils::file