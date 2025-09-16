#pragma once

#include <molcpp/io/base/trajectory.hpp>
#include <molcpp/core/frame.hpp>
#include <molcpp/types.hpp>
#include <string>
#include <iomanip>
#include <sstream>

namespace molcpp::io {

/**
 * @brief XYZ format trajectory writer
 * 
 * Writes XYZ format trajectory files. Each frame is written as:
 * - Line 1: Number of atoms
 * - Line 2: Comment line (from frame metadata or default)
 * - Lines 3..N+2: Atom data in format "element x y z"
 */
class XYZWriter : public BaseTrajectoryWriter {
public:
    using frame_type = molcpp::Frame;
    
    explicit XYZWriter(const path_type& path);

    void write_frame(const frame_type& frame) override;

private:
    std::string format_coordinate(float value) const;
    std::string get_comment_line(const frame_type& frame) const;
};

} // namespace molcpp::io