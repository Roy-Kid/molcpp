#pragma once

#include <molcpp/types.hpp>
#include <filesystem>
#include <fstream>
#include <memory>

namespace molcpp::io {

/**
 * @brief Base class for all file writers
 */
class BaseWriter {
public:
    using path_type = std::filesystem::path;

    explicit BaseWriter(const path_type& path) : path_(path) {}
    virtual ~BaseWriter() = default;

    // Non-copyable but movable
    BaseWriter(const BaseWriter&) = delete;
    BaseWriter& operator=(const BaseWriter&) = delete;
    BaseWriter(BaseWriter&&) = default;
    BaseWriter& operator=(BaseWriter&&) = default;

    const path_type& path() const { return path_; }

protected:
    path_type path_;
    std::unique_ptr<std::ofstream> file_;

    void open_file() {
        file_ = std::make_unique<std::ofstream>(path_);
        if (!file_->is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + path_.string());
        }
    }
};

} // namespace molcpp::io