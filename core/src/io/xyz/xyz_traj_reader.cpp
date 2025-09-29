#include "molcpp/io/xyz.hpp"
#include "molcpp/io/xyz/xyz_utils.hpp"
#include "molcpp/io/utils.hpp"
#include <fstream>

namespace molcpp {

XYZTrajectoryReader::XYZTrajectoryReader(const std::string& path)
    : path_(path), current_step_(0), indexed_(false) {
    file_ = std::make_unique<std::ifstream>(path);
    if (!file_->is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    load_index();
}

XYZTrajectoryReader::~XYZTrajectoryReader() = default;

std::expected<Frame, xyz_parse_error> XYZTrajectoryReader::read() {
    if (!file_ || !file_->is_open()) {
        return std::unexpected(xyz_parse_error(0, "File not open"));
    }
    auto result = parse_one_frame(*file_);
    if (result) { 
        current_step_++; 
    }
    return result;
}

std::expected<Frame, xyz_parse_error> XYZTrajectoryReader::read_step(std::size_t step) {
    if (!indexed_) {
        return std::unexpected(xyz_parse_error(0, "Trajectory not indexed"));
    }
    if (step >= index_->frame_offsets.size()) {
        return std::unexpected(xyz_parse_error(0, "Step out of range"));
    }
    
    file_->seekg(index_->frame_offsets[step]);
    auto result = parse_one_frame(*file_);
    if (result) {
        current_step_ = step + 1;
    }
    return result;
}

std::size_t XYZTrajectoryReader::tell() const { 
    return current_step_; 
}

std::size_t XYZTrajectoryReader::steps() const { 
    return indexed_ ? index_->frame_offsets.size() : 0; 
}

bool XYZTrajectoryReader::is_indexed() const { 
    return indexed_; 
}

bool XYZTrajectoryReader::build_index() {
    if (!file_ || !file_->is_open()) return false;
    
    index_ = std::make_unique<trajectory_index>();
    file_->clear();
    file_->seekg(0);
    
    std::string line;
    while (std::getline(*file_, line)) {
        if (line.empty()) continue;
        
        auto atom_count_result = to_size_t(trim(line));
        if (atom_count_result) {
            // Record the position of the start of this frame
            auto frame_start = file_->tellg() - static_cast<std::streampos>(line.length() + 1);
            index_->frame_offsets.push_back(frame_start);
            
            // Skip comment line
            std::getline(*file_, line);
            
            // Skip atom lines
            for (std::size_t i = 0; i < atom_count_result.value(); ++i) {
                std::getline(*file_, line);
            }
        }
    }
    
    indexed_ = true;
    file_->clear();
    return true;
}

bool XYZTrajectoryReader::load_index() { 
    return false; // Not implemented yet
}

bool XYZTrajectoryReader::save_index() { 
    return false; // Not implemented yet
}

xyz_trajectory_iterator XYZTrajectoryReader::begin() { 
    return xyz_trajectory_iterator(this); 
}

xyz_trajectory_sentinel XYZTrajectoryReader::end() { 
    return {}; 
}

Frame XYZTrajectoryReader::read_frame(std::size_t i) {
    if (!indexed_) build_index();
    auto res = read_step(i);
    if (!res) {
        throw std::runtime_error("Failed to read frame: " + res.error().message);
    }
    return std::move(res.value());
}

std::vector<Frame> XYZTrajectoryReader::read_all() {
    if (!indexed_) build_index();
    std::vector<Frame> out;
    out.reserve(index_->frame_offsets.size());
    for (std::size_t i = 0; i < index_->frame_offsets.size(); ++i) {
        out.push_back(read_frame(i));
    }
    return out;
}

std::vector<Frame> XYZTrajectoryReader::read_range(std::size_t start, std::size_t end, std::size_t stride) {
    if (!indexed_) build_index();
    if (end > index_->frame_offsets.size()) {
        end = index_->frame_offsets.size();
    }
    
    std::vector<Frame> out;
    for (std::size_t i = start; i < end; i += stride) {
        out.push_back(read_frame(i));
    }
    return out;
}

std::vector<Frame> XYZTrajectoryReader::read_frames(const std::vector<std::size_t>& indices) {
    if (!indexed_) build_index();
    std::vector<Frame> out;
    out.reserve(indices.size());
    for (auto i : indices) {
        out.push_back(read_frame(i));
    }
    return out;
}

} // namespace molcpp