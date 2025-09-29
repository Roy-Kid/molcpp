#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "molcpp/io/xyz.hpp"

using namespace molcpp;

TEST_CASE("XYZ parsing - real data files", "[xyz][io][real_data]") {
    
    SECTION("Parse methane.xyz") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/methane.xyz";
        std::ifstream file(file_path);
        REQUIRE(file.is_open());
        
        std::expected<Frame, xyz_parse_error> result = parse_one_frame(file);
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        // Check that frame contains atoms block
        REQUIRE(frame.contains_block("atoms"));
        
        const Block& atoms = frame["atoms"];
        REQUIRE(atoms.contains("positions"));
        
        // Check positions
        const xt::xarray<double>& positions = atoms["positions"];
        const xt::xarray<double>& positions_array = static_cast<const xt::xarray<double>&>(positions);
        
        REQUIRE(positions_array.shape().size() == 2);
        REQUIRE(positions_array.shape(0) == 5);
        REQUIRE(positions_array.shape(1) == 3);
        
        // Check methane coordinates (C at origin, 4 H atoms around it)
        REQUIRE_THAT(positions_array(0, 0), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(positions_array(0, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(positions_array(0, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
        
        REQUIRE_THAT(positions_array(1, 0), Catch::Matchers::WithinRel(0.631716, 1e-6));
        REQUIRE_THAT(positions_array(1, 1), Catch::Matchers::WithinRel(0.631716, 1e-6));
        REQUIRE_THAT(positions_array(1, 2), Catch::Matchers::WithinRel(0.631716, 1e-6));
        
        REQUIRE_THAT(positions_array(2, 0), Catch::Matchers::WithinRel(-0.631716, 1e-6));
        REQUIRE_THAT(positions_array(2, 1), Catch::Matchers::WithinRel(-0.631716, 1e-6));
        REQUIRE_THAT(positions_array(2, 2), Catch::Matchers::WithinRel(0.631716, 1e-6));
        
        REQUIRE_THAT(positions_array(3, 0), Catch::Matchers::WithinRel(0.631716, 1e-6));
        REQUIRE_THAT(positions_array(3, 1), Catch::Matchers::WithinRel(-0.631716, 1e-6));
        REQUIRE_THAT(positions_array(3, 2), Catch::Matchers::WithinRel(-0.631716, 1e-6));
        
        REQUIRE_THAT(positions_array(4, 0), Catch::Matchers::WithinRel(-0.631716, 1e-6));
        REQUIRE_THAT(positions_array(4, 1), Catch::Matchers::WithinRel(0.631716, 1e-6));
        REQUIRE_THAT(positions_array(4, 2), Catch::Matchers::WithinRel(-0.631716, 1e-6));
    }
    
    SECTION("Parse velocities.xyz with ExtXYZ format") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/velocities.xyz";
        std::ifstream file(file_path);
        REQUIRE(file.is_open());
        
        std::expected<Frame, xyz_parse_error> result = parse_one_frame(file);
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        const Block& atoms = frame["atoms"];
        REQUIRE(atoms.contains("velocities"));
        
        // Check velocities
        const xt::xarray<double>& velocities = atoms["velocities"];
        const xt::xarray<double>& velocities_array = static_cast<const xt::xarray<double>&>(velocities);
        
        REQUIRE(velocities_array.shape().size() == 2);
        REQUIRE(velocities_array.shape(0) == 2);
        REQUIRE(velocities_array.shape(1) == 3);
        
        // Check first atom velocity
        REQUIRE_THAT(velocities_array(0, 0), Catch::Matchers::WithinRel(1.0, 1e-10));
        REQUIRE_THAT(velocities_array(0, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(velocities_array(0, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
        
        // Check second atom velocity
        REQUIRE_THAT(velocities_array(1, 0), Catch::Matchers::WithinRel(-1.0, 1e-10));
        REQUIRE_THAT(velocities_array(1, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(velocities_array(1, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
    }
    
    SECTION("Parse trajectory.xyz with multiple frames") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/trajectory.xyz";
        XYZTrajectoryReader reader(file_path);
        
        // Read first frame
        std::expected<Frame, xyz_parse_error> result1 = reader.read();
        REQUIRE(result1.has_value());
        const Frame& frame1 = result1.value();
        
        const Block& atoms1 = frame1["atoms"];
        const xt::xarray<double>& positions1 = atoms1["positions"];
        const xt::xarray<double>& positions1_array = static_cast<const xt::xarray<double>&>(positions1);
        
        REQUIRE(positions1_array.shape(0) == 9);
        REQUIRE_THAT(positions1_array(0, 0), Catch::Matchers::WithinRel(0.49053, 1e-5));
        REQUIRE_THAT(positions1_array(0, 1), Catch::Matchers::WithinRel(8.41351, 1e-5));
        REQUIRE_THAT(positions1_array(0, 2), Catch::Matchers::WithinRel(0.0777257, 1e-5));
        
        // Read second frame
        std::expected<Frame, xyz_parse_error> result2 = reader.read();
        REQUIRE(result2.has_value());
        const Frame& frame2 = result2.value();
        
        const Block& atoms2 = frame2["atoms"];
        const xt::xarray<double>& positions2 = atoms2["positions"];
        
        REQUIRE(positions2.shape(0) == 9);
        REQUIRE_THAT(positions2(0, 0), Catch::Matchers::WithinRel(0.411134, 1e-5));
        REQUIRE_THAT(positions2(0, 1), Catch::Matchers::WithinRel(8.411223, 1e-5));
        REQUIRE_THAT(positions2(0, 2), Catch::Matchers::WithinRel(0.116507, 1e-5));
        
        // No more frames
        std::expected<Frame, xyz_parse_error> result3 = reader.read();
        REQUIRE_FALSE(result3.has_value());
    }
    
    SECTION("Parse extended.xyz with lattice and properties") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/extended.xyz";
        std::ifstream file(file_path);
        REQUIRE(file.is_open());
        
        std::expected<Frame, xyz_parse_error> result = parse_one_frame(file);
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        // Check atoms
        REQUIRE(frame.contains_block("atoms"));
        const Block& atoms = frame["atoms"];
        REQUIRE(atoms.contains("positions"));
        
        const xt::xarray<double>& positions = atoms["positions"];
        const xt::xarray<double>& positions_array = static_cast<const xt::xarray<double>&>(positions);
        
        REQUIRE(positions_array.shape(0) == 192);
        REQUIRE(positions_array.shape(1) == 3);
        
        // Check first atom coordinates
        REQUIRE_THAT(positions_array(0, 0), Catch::Matchers::WithinRel(2.33827271799, 1e-8));
        REQUIRE_THAT(positions_array(0, 1), Catch::Matchers::WithinRel(4.55315540425, 1e-8));
        REQUIRE_THAT(positions_array(0, 2), Catch::Matchers::WithinRel(11.5841360926, 1e-8));
        
        // Check lattice if present
        if (frame.has_metadata("lattice")) {
            const xt::xarray<double>& lattice_matrix = frame.get_metadata<xt::xarray<double>>("lattice");
            
            REQUIRE(lattice_matrix.shape(0) == 3);
            REQUIRE(lattice_matrix.shape(1) == 3);
            
            // Check lattice vectors
            REQUIRE_THAT(lattice_matrix(0, 0), Catch::Matchers::WithinRel(8.43116035, 1e-6));
            REQUIRE_THAT(lattice_matrix(0, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
            REQUIRE_THAT(lattice_matrix(0, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
        }
    }
    
    SECTION("Parse spaces.xyz with various spacing") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/spaces.xyz";
        std::ifstream file(file_path);
        REQUIRE(file.is_open());
        
        std::expected<Frame, xyz_parse_error> result = parse_one_frame(file);
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        const Block& atoms = frame["atoms"];
        const xt::xarray<double>& positions = atoms["positions"];
        const xt::xarray<double>& positions_array = static_cast<const xt::xarray<double>&>(positions);
        
        REQUIRE(positions_array.shape(0) == 64);
        REQUIRE(positions_array.shape(1) == 3);
        
        // Check first atom coordinates
        REQUIRE_THAT(positions_array(0, 0), Catch::Matchers::WithinRel(0.1668, 1e-4));
        REQUIRE_THAT(positions_array(0, 1), Catch::Matchers::WithinRel(0.1005, 1e-4));
        REQUIRE_THAT(positions_array(0, 2), Catch::Matchers::WithinRel(0.5014, 1e-4));
    }
    
    SECTION("Parse bad files - error handling") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/bad/extended-bad-properties.xyz";
        std::ifstream file(file_path);
        REQUIRE(file.is_open());
        
        // This file has multiple frames with bad properties, should fail on first frame
        std::expected<Frame, xyz_parse_error> result = parse_one_frame(file);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().line > 0);
        REQUIRE_FALSE(result.error().message.empty());
    }
}

TEST_CASE("XYZFrameReader - real data files", "[xyz][io][frame_reader]") {
    
    SECTION("Read methane.xyz using XYZFrameReader") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/methane.xyz";
        XYZFrameReader reader(file_path);
        
        std::expected<Frame, xyz_parse_error> result = reader.read();
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        const Block& atoms = frame["atoms"];
        const xt::xarray<double>& positions = atoms["positions"];
        const xt::xarray<double>& positions_array = static_cast<const xt::xarray<double>&>(positions);
        
        REQUIRE(positions_array.shape(0) == 5);
        REQUIRE(positions_array.shape(1) == 3);
        
        // Check C atom at origin
        REQUIRE_THAT(positions_array(0, 0), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(positions_array(0, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(positions_array(0, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
    }
    
    SECTION("Read velocities.xyz using XYZFrameReader") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/velocities.xyz";
        XYZFrameReader reader(file_path);
        
        std::expected<Frame, xyz_parse_error> result = reader.read();
        REQUIRE(result.has_value());
        const Frame& frame = result.value();
        
        const Block& atoms = frame["atoms"];
        REQUIRE(atoms.contains("velocities"));
        
        const xt::xarray<double>& velocities = atoms["velocities"];
        const xt::xarray<double>& velocities_array = static_cast<const xt::xarray<double>&>(velocities);
        
        REQUIRE(velocities_array.shape(0) == 2);
        REQUIRE(velocities_array.shape(1) == 3);
    }
}

TEST_CASE("XYZTrajectoryReader - real data files", "[xyz][io][trajectory_reader]") {
    
    SECTION("Read trajectory.xyz using XYZTrajectoryReader") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/trajectory.xyz";
        XYZTrajectoryReader reader(file_path);
        
        // Test sequential reading
        std::expected<Frame, xyz_parse_error> result1 = reader.read();
        REQUIRE(result1.has_value());
        
        std::expected<Frame, xyz_parse_error> result2 = reader.read();
        REQUIRE(result2.has_value());
        
        std::expected<Frame, xyz_parse_error> result3 = reader.read();
        REQUIRE_FALSE(result3.has_value());
    }
    
    SECTION("Read trajectory.xyz with indexing") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/trajectory.xyz";
        XYZTrajectoryReader reader(file_path);
        
        // Build index
        bool indexed = reader.build_index();
        REQUIRE(indexed);
        REQUIRE(reader.is_indexed());
        REQUIRE(reader.steps() == 2);
        
        // Test random access
        auto result1 = reader.read_step(0);
        REQUIRE(result1.has_value());
        
        auto result2 = reader.read_step(1);
        REQUIRE(result2.has_value());
        
        // Test out of bounds
        auto result3 = reader.read_step(2);
        REQUIRE_FALSE(result3.has_value());
    }
    
    SECTION("Read trajectory.xyz with range-for iteration") {
        std::string file_path = "/workspaces/molcrafts-1/molcpp/core/tests/data/xyz/trajectory.xyz";
        XYZTrajectoryReader reader(file_path);
        
        std::size_t frame_count = 0;
        for (const auto& frame : reader) {
            frame_count++;
            const Block& atoms = frame["atoms"];
            const xt::xarray<double>& positions = atoms["positions"];
            const xt::xarray<double>& positions_array = static_cast<const xt::xarray<double>&>(positions);
            
            REQUIRE(positions_array.shape(0) == 9);
            REQUIRE(positions_array.shape(1) == 3);
        }
        
        REQUIRE(frame_count == 2);
    }
}

TEST_CASE("XYZFrameWriter - write and read back", "[xyz][io][frame_writer]") {
    
    SECTION("Write methane-like structure and read back") {
        // Create a frame similar to methane
        molcpp::Frame frame;
        molcpp::Block atoms_block;
        
        // Create positions for methane
        auto positions = xt::xarray<double>::from_shape({5, 3});
        positions(0, 0) = 0.0; positions(0, 1) = 0.0; positions(0, 2) = 0.0;  // C
        positions(1, 0) = 0.631716; positions(1, 1) = 0.631716; positions(1, 2) = 0.631716;  // H
        positions(2, 0) = -0.631716; positions(2, 1) = -0.631716; positions(2, 2) = 0.631716;  // H
        positions(3, 0) = 0.631716; positions(3, 1) = -0.631716; positions(3, 2) = -0.631716;  // H
        positions(4, 0) = -0.631716; positions(4, 1) = 0.631716; positions(4, 2) = -0.631716;  // H
        atoms_block.set("positions", positions);
        
        frame["atoms"] = std::move(atoms_block);
        
        // Write to temporary file
        std::string temp_file = "test_methane_write.xyz";
        auto result = XYZFrameWriter::write(temp_file, frame);
        REQUIRE(result.has_value());
        
        // Read back and verify
        XYZFrameReader reader(temp_file);
        auto read_result = reader.read();
        REQUIRE(read_result.has_value());
        
        const auto& read_frame = read_result.value();
        const auto& read_atoms = read_frame["atoms"];
        const auto& read_positions = read_atoms["positions"];
        auto read_positions_array = static_cast<const xt::xarray<double>&>(read_positions);
        
        REQUIRE(read_positions_array.shape(0) == 5);
        REQUIRE(read_positions_array.shape(1) == 3);
        
        // Check C atom at origin
        REQUIRE_THAT(read_positions_array(0, 0), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(read_positions_array(0, 1), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(read_positions_array(0, 2), Catch::Matchers::WithinRel(0.0, 1e-10));
        
        // Clean up
        std::filesystem::remove(temp_file);
    }
}

TEST_CASE("XYZTrajectoryWriter - write trajectory", "[xyz][io][trajectory_writer]") {
    
    SECTION("Write multiple frames and read back") {
        std::string temp_file = "test_trajectory_write.xyz";
        
        // Create trajectory writer
        XYZTrajectoryWriter writer(temp_file);
        REQUIRE(writer.is_open());
        
        // Write first frame
        molcpp::Frame frame1;
        molcpp::Block atoms1_block;
        auto positions1 = xt::xarray<double>::from_shape({2, 3});
        positions1(0, 0) = 0.0; positions1(0, 1) = 0.0; positions1(0, 2) = 0.0;
        positions1(1, 0) = 1.0; positions1(1, 1) = 0.0; positions1(1, 2) = 0.0;
        atoms1_block.set("positions", positions1);
        frame1["atoms"] = std::move(atoms1_block);
        
        auto result1 = writer.write(frame1);
        REQUIRE(result1.has_value());
        
        // Write second frame
        molcpp::Frame frame2;
        molcpp::Block atoms2_block;
        auto positions2 = xt::xarray<double>::from_shape({2, 3});
        positions2(0, 0) = 0.1; positions2(0, 1) = 0.0; positions2(0, 2) = 0.0;
        positions2(1, 0) = 1.1; positions2(1, 1) = 0.0; positions2(1, 2) = 0.0;
        atoms2_block.set("positions", positions2);
        frame2["atoms"] = std::move(atoms2_block);
        
        auto result2 = writer.write(frame2);
        REQUIRE(result2.has_value());
        
        writer.close();
        REQUIRE_FALSE(writer.is_open());
        
        // Read back and verify
        XYZTrajectoryReader reader(temp_file);
        
        auto read_result1 = reader.read();
        REQUIRE(read_result1.has_value());
        const auto& read_frame1 = read_result1.value();
        const auto& read_atoms1 = read_frame1["atoms"];
        const auto& read_positions1 = read_atoms1["positions"];
        auto read_positions1_array = static_cast<const xt::xarray<double>&>(read_positions1);
        
        REQUIRE_THAT(read_positions1_array(0, 0), Catch::Matchers::WithinRel(0.0, 1e-10));
        REQUIRE_THAT(read_positions1_array(1, 0), Catch::Matchers::WithinRel(1.0, 1e-10));
        
        auto read_result2 = reader.read();
        REQUIRE(read_result2.has_value());
        const auto& read_frame2 = read_result2.value();
        const auto& read_atoms2 = read_frame2["atoms"];
        const auto& read_positions2 = read_atoms2["positions"];
        auto read_positions2_array = static_cast<const xt::xarray<double>&>(read_positions2);
        
        REQUIRE_THAT(read_positions2_array(0, 0), Catch::Matchers::WithinRel(0.1, 1e-10));
        REQUIRE_THAT(read_positions2_array(1, 0), Catch::Matchers::WithinRel(1.1, 1e-10));
        
        // Clean up
        std::filesystem::remove(temp_file);
    }
}

TEST_CASE("XYZ parsing - error handling", "[xyz][io][error_handling]") {
    
    SECTION("Parse non-existent file") {
        XYZFrameReader reader("non_existent_file.xyz");
        std::expected<Frame, xyz_parse_error> result = reader.read();
        REQUIRE_FALSE(result.has_value());
    }
    
    SECTION("Parse empty file") {
        std::string temp_file = "test_empty.xyz";
        std::ofstream out(temp_file);
        out.close();
        
        XYZFrameReader reader(temp_file);
        std::expected<Frame, xyz_parse_error> result = reader.read();
        REQUIRE_FALSE(result.has_value());
        
        std::filesystem::remove(temp_file);
    }
    
    SECTION("Parse malformed file") {
        std::string temp_file = "test_malformed.xyz";
        std::ofstream out(temp_file);
        out << "invalid\ncomment\nH 0.0 0.0 0.0\n";
        out.close();
        
        XYZFrameReader reader(temp_file);
        std::expected<Frame, xyz_parse_error> result = reader.read();
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().line > 0);
        REQUIRE_FALSE(result.error().message.empty());
        
        std::filesystem::remove(temp_file);
    }
}
