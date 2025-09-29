#include "molcpp/io/xyz.hpp"
#include "molcpp/io/xyz/xyz_types.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace molcpp {

XYZFrameReader::XYZFrameReader(const std::string& path) : path_(path) {
    file_ = std::make_unique<std::ifstream>(path);
    // Don't throw exception here, check in read() method instead
}

XYZFrameReader::~XYZFrameReader() = default;

std::expected<Frame, xyz_parse_error> XYZFrameReader::read() {
    if (!file_->is_open()) {
        return std::unexpected(xyz_parse_error(0, "Cannot open file: " + path_));
    }
    
    return parse_one_frame(*file_);
}

std::expected<Frame, xyz_parse_error> XYZFrameReader::from_string(std::string_view text) {
    std::istringstream stream{std::string(text)};
    return parse_one_frame(stream);
}

} // namespace molcpp