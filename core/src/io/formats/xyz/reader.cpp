#include <molcpp/io/formats/xyz/reader.hpp>
#include <molcpp/io/formats/xyz/parser.hpp>
#include <sstream>

namespace molcpp::io {

XYZReader::XYZReader(const path_type& path)
    : BaseTrajectoryReader(path) {}

XYZReader::XYZReader(const std::vector<path_type>& paths)
    : BaseTrajectoryReader(paths) {}

void XYZReader::parse_trajectory(size_t file_index) {
    auto& file = files_[file_index];
    const auto& path = paths_[file_index];
    
    file->seekg(0);
    
    size_t current_offset = 0;
    std::string line;
    
    while (std::getline(*file, line)) {
        try {
            int n_atoms = std::stoi(line);
            if (n_atoms > 0) {
                FrameLocation location{ file_index, current_offset, path };
                frame_locations_.push_back(location);
                
                // Skip comment line
                std::getline(*file, line);
                current_offset = file->tellg();
                
                // Skip atom lines
                for (int i = 0; i < n_atoms; ++i) {
                    std::getline(*file, line);
                    current_offset = file->tellg();
                }
            } else {
                current_offset = file->tellg();
            }
        } catch (const std::exception&) {
            current_offset = file->tellg();
        }
    }
    total_frames_ = frame_locations_.size();
}

XYZReader::frame_type XYZReader::parse_frame(const std::vector<std::string>& lines) {
    return xyz_parser::parse_xyz_frame(lines);
}

XYZReader::AtomData XYZReader::parse_atom_line(const std::string& line) const {
    std::istringstream iss(line);
    AtomData data;
    
    float x, y, z;
    if (!(iss >> data.element >> x >> y >> z)) {
        throw std::runtime_error("Invalid atom line format");
    }
    
    data.coordinates = {x, y, z};
    
    // Get any remaining data as comment
    std::string remaining;
    std::getline(iss, remaining);
    data.comment = remaining;
    
    return data;
}

} // namespace molcpp::io