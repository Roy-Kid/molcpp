#pragma once

#include <molcpp/types.hpp>
#include <filesystem>
#include <memory>

namespace molcpp::io {

/**
 * @brief Base class for all file readers
 */
class BaseReader {
public:
    using path_type = std::filesystem::path;

    explicit BaseReader(const path_type& path) : path_(path) {}
    virtual ~BaseReader() = default;

    // Non-copyable but movable
    BaseReader(const BaseReader&) = delete;
    BaseReader& operator=(const BaseReader&) = delete;
    BaseReader(BaseReader&&) = default;
    BaseReader& operator=(BaseReader&&) = default;

    const path_type& path() const { return path_; }

protected:
    path_type path_;
};

} // namespace molcpp::io