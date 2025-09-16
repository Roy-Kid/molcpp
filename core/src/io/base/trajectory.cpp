#include <molcpp/io/base/trajectory.hpp>
#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace molcpp::io {

BaseTrajectoryReader::BaseTrajectoryReader(const path_type& path)
    : BaseReader(path), paths_({path}), total_frames_(0) { 
    open_files(); 
}

BaseTrajectoryReader::BaseTrajectoryReader(const std::vector<path_type>& paths)
    : BaseReader(paths.empty() ? path_type{} : paths[0]), paths_(paths), total_frames_(0) { 
    open_files(); 
}

BaseTrajectoryReader::~BaseTrajectoryReader() { 
    close(); 
}

BaseTrajectoryReader::BaseTrajectoryReader(BaseTrajectoryReader&& other) noexcept
    : BaseReader(std::move(other))
    , paths_(std::move(other.paths_))
    , files_(std::move(other.files_))
    , frame_locations_(std::move(other.frame_locations_))
    , total_frames_(other.total_frames_) { 
    other.total_frames_ = 0; 
}

BaseTrajectoryReader& BaseTrajectoryReader::operator=(BaseTrajectoryReader&& other) noexcept {
    if (this != &other) {
        close();
        BaseReader::operator=(std::move(other));
        paths_ = std::move(other.paths_);
        files_ = std::move(other.files_);
        frame_locations_ = std::move(other.frame_locations_);
        total_frames_ = other.total_frames_;
        other.total_frames_ = 0;
    }
    return *this;
}

void BaseTrajectoryReader::open_files() {
    files_.clear();
    frame_locations_.clear();
    for (size_t i = 0; i < paths_.size(); ++i) {
        const auto& path = paths_[i];
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("File not found: " + path.string());
        }
        auto file = std::make_unique<std::ifstream>(path, std::ios::binary);
        if (!file->is_open()) {
            throw std::runtime_error("Cannot open file: " + path.string());
        }
        files_.push_back(std::move(file));
        parse_trajectory(i);
    }
}

BaseTrajectoryReader::frame_type BaseTrajectoryReader::read_frame(size_t index) {
    if (index >= total_frames_) { 
        throw std::out_of_range("Frame index out of range"); 
    }
    const auto location = get_frame_location(index);
    auto& file = files_[location.file_index];
    file->seekg(location.byte_offset);

    std::vector<std::string> frame_lines; 
    std::string line;
    if (!std::getline(*file, line)) {
        throw std::runtime_error("Failed to read atom count line");
    }
    frame_lines.push_back(line);
    if (!std::getline(*file, line)) {
        throw std::runtime_error("Failed to read comment line");
    }
    frame_lines.push_back(line);
    int n_atoms = std::stoi(frame_lines[0]);
    for (int i = 0; i < n_atoms; ++i) {
        if (!std::getline(*file, line)) {
            throw std::runtime_error("Failed to read atom line " + std::to_string(i));
        }
        frame_lines.push_back(line);
    }
    return parse_frame(frame_lines);
}

std::vector<BaseTrajectoryReader::frame_type> BaseTrajectoryReader::read_frames(const std::vector<size_t>& indices) {
    std::vector<frame_type> frames; 
    frames.reserve(indices.size());
    for (auto idx : indices) {
        frames.push_back(read_frame(idx));
    }
    return frames;
}

std::vector<BaseTrajectoryReader::frame_type> BaseTrajectoryReader::read_range(size_t start, size_t stop, size_t step) {
    std::vector<frame_type> frames; 
    for (size_t i = start; i < stop; i += step) {
        if (i < total_frames_) {
            frames.push_back(read_frame(i));
        }
    }
    return frames; 
}

std::vector<BaseTrajectoryReader::frame_type> BaseTrajectoryReader::read_all() {
    std::vector<frame_type> frames; 
    frames.reserve(total_frames_); 
    for (size_t i = 0; i < total_frames_; ++i) {
        frames.push_back(read_frame(i));
    }
    return frames; 
}

size_t BaseTrajectoryReader::n_frames() const { 
    return total_frames_; 
}

bool BaseTrajectoryReader::empty() const { 
    return total_frames_ == 0; 
}

void BaseTrajectoryReader::close() { 
    files_.clear(); 
    frame_locations_.clear(); 
    total_frames_ = 0; 
}

FrameLocation BaseTrajectoryReader::get_frame_location(size_t index) const { 
    if (index >= frame_locations_.size()) {
        throw std::out_of_range("Frame location index out of range"); 
    }
    return frame_locations_[index]; 
}

BaseTrajectoryReader::Iterator BaseTrajectoryReader::begin() { 
    return Iterator(this, 0); 
}

BaseTrajectoryReader::Iterator BaseTrajectoryReader::end() { 
    return Iterator(this, total_frames_); 
}

BaseTrajectoryReader::Iterator::Iterator(BaseTrajectoryReader* reader, size_t index) 
    : reader_(reader), index_(index) {}

BaseTrajectoryReader::frame_type& BaseTrajectoryReader::Iterator::operator*() { 
    if (!current_frame_.has_value()) {
        current_frame_ = reader_->read_frame(index_); 
    }
    return current_frame_.value(); 
}

BaseTrajectoryReader::frame_type* BaseTrajectoryReader::Iterator::operator->() { 
    if (!current_frame_.has_value()) {
        current_frame_ = reader_->read_frame(index_); 
    }
    return &current_frame_.value(); 
}

BaseTrajectoryReader::Iterator& BaseTrajectoryReader::Iterator::operator++() { 
    ++index_; 
    current_frame_.reset(); 
    return *this; 
}

BaseTrajectoryReader::Iterator BaseTrajectoryReader::Iterator::operator++(int) { 
    Iterator tmp = *this; 
    ++(*this); 
    return tmp; 
}

bool BaseTrajectoryReader::Iterator::operator==(const Iterator& other) const { 
    return reader_ == other.reader_ && index_ == other.index_; 
}

bool BaseTrajectoryReader::Iterator::operator!=(const Iterator& other) const { 
    return !(*this == other); 
}

// BaseTrajectoryWriter implementation
BaseTrajectoryWriter::BaseTrajectoryWriter(const path_type& path) 
    : BaseWriter(path) {
    open_file();
}

BaseTrajectoryWriter::~BaseTrajectoryWriter() { 
    close(); 
}

BaseTrajectoryWriter::BaseTrajectoryWriter(BaseTrajectoryWriter&& other) noexcept
    : BaseWriter(std::move(other)) {}

BaseTrajectoryWriter& BaseTrajectoryWriter::operator=(BaseTrajectoryWriter&& other) noexcept { 
    if (this != &other) { 
        close(); 
        BaseWriter::operator=(std::move(other));
    } 
    return *this; 
}

void BaseTrajectoryWriter::close() { 
    if (file_ && file_->is_open()) {
        file_->close(); 
    }
}

} // namespace molcpp::io