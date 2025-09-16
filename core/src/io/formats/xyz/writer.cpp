#include <molcpp/io/formats/xyz/writer.hpp>
#include <molcpp/core/element.hpp>
#include <iomanip>

namespace molcpp::io {

XYZWriter::XYZWriter(const path_type& path)
    : BaseTrajectoryWriter(path) {}

void XYZWriter::write_frame(const frame_type& frame) {
    if (!file_ || !file_->is_open()) {
        throw std::runtime_error("File is not open for writing");
    }
    
    if (!frame.contains_block("atoms")) {
        throw std::runtime_error("Frame must contain 'atoms' block");
    }
    
    const molcpp::Block& atoms_block = frame["atoms"];
    if (!atoms_block.contains("coordinates")) {
        throw std::runtime_error("Atoms block must contain 'coordinates'");
    }
    
    const xt::xarray<float>& coordinates = static_cast<const xt::xarray<float>&>(atoms_block["coordinates"]);
    size_t n_atoms = coordinates.shape(0);
    
    // Get element symbols
    std::vector<std::string> elements;
    if (atoms_block.contains("atomic_numbers")) {
        const xt::xarray<float>& atomic_numbers = static_cast<const xt::xarray<float>&>(atoms_block["atomic_numbers"]);
        elements.reserve(n_atoms);
        for (size_t i = 0; i < n_atoms; ++i) {
            try {
                int atomic_number = static_cast<int>(atomic_numbers(i));
                auto element_data = Element::create(atomic_number);
                elements.push_back(element_data.symbol);
            } catch (const std::exception&) {
                elements.push_back("X");  // Unknown element
            }
        }
    } else {
        elements.resize(n_atoms, "C");  // Default to carbon
    }
    
    // Write header
    *file_ << n_atoms << "\n";
    *file_ << get_comment_line(frame) << "\n";
    
    // Write atoms
    for (size_t i = 0; i < n_atoms; ++i) {
        *file_ << std::left << std::setw(3) << elements[i];
        *file_ << std::right << std::fixed << std::setprecision(6);
        *file_ << std::setw(15) << coordinates(i, 0);
        *file_ << std::setw(15) << coordinates(i, 1);
        *file_ << std::setw(15) << coordinates(i, 2);
        *file_ << "\n";
    }
    file_->flush();
}

std::string XYZWriter::format_coordinate(float value) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << std::setw(15) << value;
    return oss.str();
}

std::string XYZWriter::get_comment_line(const frame_type& frame) const {
    try {
        return frame.get_metadata<std::string>("comment");
    } catch (...) {
        return "Frame";  // Default comment
    }
}

} // namespace molcpp::io