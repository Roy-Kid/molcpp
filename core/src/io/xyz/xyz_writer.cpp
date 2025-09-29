#include "molcpp/io/xyz.hpp"
#include "molcpp/io/xyz/xyz_utils.hpp"
#include <xtensor/containers/xarray.hpp>
#include <fstream>

namespace molcpp {

std::expected<void, std::string> XYZFrameWriter::write(const std::string& path, const Frame& frame) {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return std::unexpected("Cannot open file for writing: " + path);
    }
    return write(file, frame);
}

std::expected<void, std::string> XYZFrameWriter::write(std::ostream& out, const Frame& frame) {
    if (!frame.contains_block("atoms")) {
        return std::unexpected("Frame does not contain atoms block");
    }
    
    const Block& atoms = frame["atoms"];
    if (!atoms.contains("positions")) {
        return std::unexpected("Atoms block must contain positions");
    }
    
    const auto& positions = static_cast<const xt::xarray<double>&>(atoms["positions"]);
    if (positions.shape().size()!=2 || positions.shape(1)!=3) {
        return std::unexpected("Positions must be Nx3 array");
    }
    
    std::size_t num_atoms = positions.shape(0);
    out << num_atoms << '\n';
    out << build_extxyz_comment(frame) << '\n';
    
    // Get atomic numbers
    const xt::xarray<int>* atomic_numbers_ptr = nullptr;
    if (atoms.contains("atomic_numbers")) {
        atomic_numbers_ptr = &static_cast<const xt::xarray<int>&>(atoms["atomic_numbers"]);
    }
    
    // Get velocities if present
    const xt::xarray<double>* velocities_ptr = nullptr;
    if (atoms.contains("velocities")) {
        velocities_ptr = &static_cast<const xt::xarray<double>&>(atoms["velocities"]);
    }
    
    for (std::size_t i = 0; i < num_atoms; ++i) {
        // Convert atomic number to symbol (simple implementation)
        std::string symbol = "H"; // default
        if (atomic_numbers_ptr && i < atomic_numbers_ptr->size()) {
            int atomic_num = (*atomic_numbers_ptr)(i);
            symbol = atomic_number_to_symbol(atomic_num);
        }
        
        out << symbol << ' ' 
            << positions(i, 0) << ' ' 
            << positions(i, 1) << ' ' 
            << positions(i, 2);
        
        // Write velocities if present
        if (velocities_ptr && 
            velocities_ptr->shape().size() == 2 && 
            velocities_ptr->shape(0) == num_atoms && 
            velocities_ptr->shape(1) == 3) {
            out << ' ' << (*velocities_ptr)(i, 0) 
                << ' ' << (*velocities_ptr)(i, 1) 
                << ' ' << (*velocities_ptr)(i, 2);
        }
        
        out << '\n';
    }
    
    return {};
}

XYZTrajectoryWriter::XYZTrajectoryWriter(const std::string& path) : path_(path), frame_count_(0) {
    file_ = std::make_unique<std::ofstream>(path, std::ios::app);
    if (!file_ || !file_->is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }
}

XYZTrajectoryWriter::~XYZTrajectoryWriter() = default;

bool XYZTrajectoryWriter::is_open() const { 
    return file_ && file_->is_open(); 
}

std::expected<void, std::string> XYZTrajectoryWriter::write(const Frame& frame) {
    if (!is_open()) {
        return std::unexpected("Writer not open");
    }
    auto result = XYZFrameWriter::write(*file_, frame);
    if (result) frame_count_++;
    return result;
}

void XYZTrajectoryWriter::close() { 
    if (file_) file_->close(); 
}

} // namespace molcpp
