#include "molcpp/io/xyz.hpp"
#include "molcpp/io/xyz/xyz_utils.hpp"
#include "molcpp/io/xyz/xyz_types.hpp"
#include "molcpp/io/utils.hpp"
#include <xtensor/containers/xarray.hpp>
#include <sstream>
#include <algorithm>

namespace molcpp {

std::expected<Frame, xyz_parse_error> parse_one_frame(std::istream& in) {
    try {
        std::string line;
        if (!std::getline(in, line)) {
            return std::unexpected(xyz_parse_error(1, "Empty file"));
        }
        
        auto atom_count_result = to_size_t(trim(line));
        if (!atom_count_result) {
            return std::unexpected(xyz_parse_error(1, atom_count_result.error().message));
        }
        std::size_t atom_count = atom_count_result.value();
        
        if (atom_count == 0) {
            return std::unexpected(xyz_parse_error(1, "Atom count cannot be zero"));
        }
        
        if (!std::getline(in, line)) {
            return std::unexpected(xyz_parse_error(2, "Missing comment line"));
        }
        
        extxyz_metadata metadata;
        metadata.raw_comment = line;
        
        if (looks_like_extxyz_comment(line)) {
            metadata.is_extxyz = true;
            auto kv_pairs = parse_extxyz_kv(line);
            
            auto fetch = [&](const char* k, std::string& dst) {
                if (kv_pairs.contains(k)) dst = kv_pairs[k];
            };
            fetch("Lattice", metadata.lattice_str);
            fetch("pbc", metadata.pbc_str);
            fetch("Properties", metadata.properties_str);
            fetch("Time", metadata.time_str);
            fetch("Step", metadata.step_str);
            
            if (!metadata.properties_str.empty() && !validate_properties_format(metadata.properties_str)) {
                return std::unexpected(xyz_parse_error(2, "Invalid Properties format: " + metadata.properties_str));
            }
            
            for (auto& [k, v] : kv_pairs) {
                if (k == "Lattice" || k == "pbc" || k == "Properties" || k == "Time" || k == "Step") continue;
                metadata.extra_meta[k] = v;
            }
        }
        
        std::vector<atom_data> atoms;
        atoms.reserve(atom_count);
        
        bool has_vel = metadata.is_extxyz && has_velocities(metadata.properties_str);
        
        for (std::size_t i = 0; i < atom_count; ++i) {
            if (!std::getline(in, line)) {
                return std::unexpected(xyz_parse_error(3 + i, "Insufficient atom lines"));
            }
            
            auto tokens = split_whitespace(trim(line));
            if (tokens.size() < 4) {
                return std::unexpected(xyz_parse_error(3 + i, "Invalid atom line format"));
            }
            
            atom_data a;
            a.symbol = tokens[0];
            
            auto xres = to_double(tokens[1]); if (!xres) return std::unexpected(xyz_parse_error(3 + i, "Invalid x coordinate")); a.x = xres.value();
            auto yres = to_double(tokens[2]); if (!yres) return std::unexpected(xyz_parse_error(3 + i, "Invalid y coordinate")); a.y = yres.value();
            auto zres = to_double(tokens[3]); if (!zres) return std::unexpected(xyz_parse_error(3 + i, "Invalid z coordinate")); a.z = zres.value();
            
            if (has_vel && tokens.size() >= 7) {
                auto vxr = to_double(tokens[4]); if (!vxr) return std::unexpected(xyz_parse_error(3 + i, "Invalid vx velocity")); a.vx = vxr.value();
                auto vyr = to_double(tokens[5]); if (!vyr) return std::unexpected(xyz_parse_error(3 + i, "Invalid vy velocity")); a.vy = vyr.value();
                auto vzr = to_double(tokens[6]); if (!vzr) return std::unexpected(xyz_parse_error(3 + i, "Invalid vz velocity")); a.vz = vzr.value();
                a.has_velocity = true;
            }
            
            atoms.push_back(a);
        }
        
        Frame frame;
        Block atoms_block;
        
        auto positions = xt::xarray<double>::from_shape({atom_count, std::size_t(3)});
        auto atomic_numbers = xt::xarray<int>::from_shape({atom_count});
        
        for (std::size_t i = 0; i < atom_count; ++i) {
            positions(i, 0) = atoms[i].x;
            positions(i, 1) = atoms[i].y;
            positions(i, 2) = atoms[i].z;
            atomic_numbers(i) = symbol_to_atomic_number(atoms[i].symbol);
        }
        
        atoms_block.set("positions", positions);
        atoms_block.set("atomic_numbers", atomic_numbers);
        
        if (has_vel) {
            auto velocities = xt::xarray<double>::from_shape({atom_count, std::size_t(3)});
            for (std::size_t i = 0; i < atom_count; ++i) {
                velocities(i, 0) = atoms[i].vx;
                velocities(i, 1) = atoms[i].vy;
                velocities(i, 2) = atoms[i].vz;
            }
            atoms_block.set("velocities", velocities);
        }
        
        frame["atoms"] = std::move(atoms_block);
        
        if (metadata.is_extxyz && !metadata.lattice_str.empty()) {
            auto lattice_result = parse_lattice_matrix(metadata.lattice_str);
            if (lattice_result) frame.set_metadata("lattice", lattice_result.value());
            else return std::unexpected(xyz_parse_error(2, "Invalid Lattice format: " + lattice_result.error().message));
        }
        
        if (metadata.is_extxyz && !metadata.pbc_str.empty()) {
            auto pbc_result = parse_pbc_flags(metadata.pbc_str);
            if (pbc_result) frame.set_metadata("pbc", pbc_result.value());
            else return std::unexpected(xyz_parse_error(2, "Invalid pbc format: " + pbc_result.error().message));
        }
        
        if (metadata.is_extxyz && !metadata.time_str.empty()) {
            auto time_result = parse_time(metadata.time_str);
            if (time_result) frame.set_metadata("time", time_result.value());
            else return std::unexpected(xyz_parse_error(2, "Invalid Time format: " + time_result.error().message));
        }
        
        if (metadata.is_extxyz && !metadata.step_str.empty()) {
            auto step_result = parse_step(metadata.step_str);
            if (step_result) frame.set_metadata("step", step_result.value());
            else return std::unexpected(xyz_parse_error(2, "Invalid Step format: " + step_result.error().message));
        }
        
        for (auto& [k, v] : metadata.extra_meta) frame.set_metadata(k, v);
        frame.set_metadata("comment", metadata.raw_comment);
        
        return frame;
    } catch (const std::exception& e) {
        return std::unexpected(xyz_parse_error(0, e.what()));
    }
}

Frame parse_one_frame_throw(std::istream& in) {
    auto res = parse_one_frame(in);
    if (!res) {
        throw std::runtime_error("Parse error at line " + std::to_string(res.error().line) + ": " + res.error().message);
    }
    return res.value();
}


} // namespace molcpp
