#pragma once

#include <molcpp/io/base/trajectory.hpp>
#include <molcpp/core/frame.hpp>
#include <molcpp/types.hpp>
#include <string>
#include <vector>
#include <array>

namespace molcpp::io {

/**
 * @brief XYZ format trajectory reader
 * 
 * Reads XYZ format trajectory files. Each frame in an XYZ file consists of:
 * - Line 1: Number of atoms
 * - Line 2: Comment line
 * - Lines 3..N+2: Atom data (element x y z [additional columns])
 */
class XYZReader : public BaseTrajectoryReader {
public:
    using frame_type = molcpp::Frame;
    
    explicit XYZReader(const path_type& path);
    explicit XYZReader(const std::vector<path_type>& paths);

protected:
    frame_type parse_frame(const std::vector<std::string>& lines) override;
    void parse_trajectory(size_t file_index) override;

private:
    struct AtomData {
        std::string element;
        std::array<float, 3> coordinates;
        std::string comment;
    };

    AtomData parse_atom_line(const std::string& line) const;
};

} // namespace molcpp::io