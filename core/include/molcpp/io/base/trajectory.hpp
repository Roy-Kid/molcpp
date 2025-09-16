#pragma once

#include <molcpp/core/frame.hpp>
#include <molcpp/io/base/reader.hpp>
#include <molcpp/io/base/writer.hpp>
#include <filesystem>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <optional>

namespace molcpp::io {

struct FrameLocation {
    size_t file_index;
    size_t byte_offset;
    std::filesystem::path file_path;
};

class BaseTrajectoryReader : public BaseReader {
public:
    using frame_type = molcpp::Frame;

    explicit BaseTrajectoryReader(const path_type& path);
    explicit BaseTrajectoryReader(const std::vector<path_type>& paths);
    virtual ~BaseTrajectoryReader();

    BaseTrajectoryReader(const BaseTrajectoryReader&) = delete;
    BaseTrajectoryReader& operator=(const BaseTrajectoryReader&) = delete;
    BaseTrajectoryReader(BaseTrajectoryReader&&) noexcept;
    BaseTrajectoryReader& operator=(BaseTrajectoryReader&&) noexcept;

    class Iterator;
    Iterator begin();
    Iterator end();

    frame_type read_frame(size_t index);
    std::vector<frame_type> read_frames(const std::vector<size_t>& indices);
    std::vector<frame_type> read_range(size_t start, size_t stop, size_t step = 1);
    std::vector<frame_type> read_all();

    size_t n_frames() const;
    bool empty() const;

    void close();

protected:
    virtual frame_type parse_frame(const std::vector<std::string>& lines) = 0;
    virtual void parse_trajectory(size_t file_index) = 0;

protected:
    void open_files();
    FrameLocation get_frame_location(size_t index) const;

    std::vector<path_type> paths_;
    std::vector<std::unique_ptr<std::ifstream>> files_;
    std::vector<FrameLocation> frame_locations_;
    size_t total_frames_{};
};

class BaseTrajectoryReader::Iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = frame_type;
    using difference_type = std::ptrdiff_t;
    using pointer = frame_type*;
    using reference = frame_type&;

    Iterator(BaseTrajectoryReader* reader, size_t index);

    reference operator*();
    pointer operator->();
    Iterator& operator++();
    Iterator operator++(int);
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

private:
    BaseTrajectoryReader* reader_;
    size_t index_;
    mutable std::optional<frame_type> current_frame_;
};

class BaseTrajectoryWriter : public BaseWriter {
public:
    using frame_type = molcpp::Frame;

    explicit BaseTrajectoryWriter(const path_type& path);
    virtual ~BaseTrajectoryWriter();

    BaseTrajectoryWriter(const BaseTrajectoryWriter&) = delete;
    BaseTrajectoryWriter& operator=(const BaseTrajectoryWriter&) = delete;
    BaseTrajectoryWriter(BaseTrajectoryWriter&&) noexcept;
    BaseTrajectoryWriter& operator=(BaseTrajectoryWriter&&) noexcept;

    virtual void write_frame(const frame_type& frame) = 0;
    void close();
};

} // namespace molcpp::io