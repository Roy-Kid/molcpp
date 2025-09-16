#include <catch2/catch_test_macros.hpp>
#include <molcpp/io/formats/xyz/reader.hpp>
#include <molcpp/io/formats/xyz/writer.hpp>
#include <molcpp/core/frame.hpp>
#include <molcpp/core/element.hpp>
#include <filesystem>
#include <fstream>
#include <cmath>

using namespace molcpp::io;

// Helper function for approximate equality
template<typename T>
bool approx_equal(T a, T b, T tolerance = 1e-6) {
    return std::abs(a - b) < tolerance;
}

// Helper to create test data directory
std::filesystem::path get_test_data_dir() {
    // For now, assume test data is in the same directory as the test or skip tests that need data
    return std::filesystem::current_path() / "test_data" / "xyz";
}

// Helper to create temporary test file
std::filesystem::path create_temp_xyz_file(const std::string& content) {
    auto temp_file = std::filesystem::temp_directory_path() / "test_xyz_temp.xyz";
    std::ofstream file(temp_file);
    file << content;
    file.close();
    return temp_file;
}

TEST_CASE("XYZ Reader functionality", "[io][formats][xyz][reader]") {
    
    SECTION("Read simple XYZ file") {
        std::string xyz_content = 
            "2\n"
            "Water molecule\n"
            "O  0.000  0.000  0.000\n"
            "H  0.757  0.587  0.000\n";
        
        auto temp_file = create_temp_xyz_file(xyz_content);
        XYZReader reader(temp_file);
        
        CHECK(reader.n_frames() == 1);
        CHECK_FALSE(reader.empty());
        
        auto frame = reader.read_frame(0);
        CHECK(frame.get_metadata<std::string>("comment") == "Water molecule");
        
        const auto& atoms = frame["atoms"];
        const auto& coords = atoms.get<xt::xarray<float>>("coordinates");
        const auto& atomic_nums = atoms.get<xt::xarray<float>>("atomic_numbers");
        
        CHECK(coords.shape()[0] == 2);
        CHECK(atomic_nums(0) == 8.0);  // Oxygen
        CHECK(atomic_nums(1) == 1.0);  // Hydrogen
        
        std::filesystem::remove(temp_file);
    }
    
    SECTION("Read multi-frame trajectory") {
        std::string xyz_content = 
            "2\n"
            "Frame 1\n"
            "C  0.000  0.000  0.000\n"
            "C  1.000  0.000  0.000\n"
            "2\n"
            "Frame 2\n"
            "C  0.000  0.000  0.000\n"
            "C  1.100  0.000  0.000\n";
        
        auto temp_file = create_temp_xyz_file(xyz_content);
        XYZReader reader(temp_file);
        
        CHECK(reader.n_frames() == 2);
        
        auto frame1 = reader.read_frame(0);
        auto frame2 = reader.read_frame(1);
        
        CHECK(frame1.get_metadata<std::string>("comment") == "Frame 1");
        CHECK(frame2.get_metadata<std::string>("comment") == "Frame 2");
        
        // Check coordinate differences
        const auto& atoms1 = frame1["atoms"];
        const auto& atoms2 = frame2["atoms"];
        const auto& coords1 = atoms1.get<xt::xarray<float>>("coordinates");
        const auto& coords2 = atoms2.get<xt::xarray<float>>("coordinates");
        
        CHECK(approx_equal(coords1(1, 0), 1.0));
        CHECK(approx_equal(coords2(1, 0), 1.1));
        
        std::filesystem::remove(temp_file);
    }
    
    SECTION("Read all frames") {
        std::string xyz_content = 
            "1\nFrame 1\nC  0.0  0.0  0.0\n"
            "1\nFrame 2\nC  1.0  0.0  0.0\n"
            "1\nFrame 3\nC  2.0  0.0  0.0\n";
        
        auto temp_file = create_temp_xyz_file(xyz_content);
        XYZReader reader(temp_file);
        
        auto all_frames = reader.read_all();
        CHECK(all_frames.size() == 3);
        
        // Test range reading
        auto range_frames = reader.read_range(0, 2, 1);
        CHECK(range_frames.size() == 2);
        
        // Test specific frames
        auto specific_frames = reader.read_frames({0, 2});
        CHECK(specific_frames.size() == 2);
        
        std::filesystem::remove(temp_file);
    }
}

TEST_CASE("XYZ Writer functionality", "[io][formats][xyz][writer]") {
    
    SECTION("Write simple frame") {
        auto temp_file = std::filesystem::temp_directory_path() / "test_write.xyz";
        
        // Create a frame
        molcpp::Frame frame;
        molcpp::Block atoms;
        
        // Set coordinates
        xt::xarray<float> coords = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}};
        atoms.set("coordinates", coords);
        
        // Set atomic numbers
        xt::xarray<float> atomic_nums = {6.0, 1.0};  // C, H
        atoms.set("atomic_numbers", atomic_nums);
        
        frame.set_block("atoms", atoms);
        frame.set_metadata("comment", "Test molecule");
        
        // Write frame
        XYZWriter writer(temp_file);
        writer.write_frame(frame);
        writer.close();
        
        // Read back and verify
        std::ifstream file(temp_file);
        std::string line;
        
        std::getline(file, line);
        CHECK(line == "2");
        
        std::getline(file, line);
        CHECK(line == "Test molecule");
        
        std::getline(file, line);
        CHECK(line.find("C") == 0);
        CHECK(line.find("0.000000") != std::string::npos);
        
        file.close();
        std::filesystem::remove(temp_file);
    }
    
    SECTION("Write and read roundtrip") {
        auto temp_file = std::filesystem::temp_directory_path() / "test_roundtrip.xyz";
        
        // Create original frame
        molcpp::Frame original_frame;
        molcpp::Block atoms;
        
        xt::xarray<float> coords = {{0.0, 0.0, 0.0}, {1.234, -2.567, 3.890}};
        xt::xarray<float> atomic_nums = {8.0, 1.0};  // O, H
        
        atoms.set("coordinates", coords);
        atoms.set("atomic_numbers", atomic_nums);
        original_frame.set_block("atoms", atoms);
        original_frame.set_metadata("comment", "Roundtrip test");
        
        // Write frame
        XYZWriter writer(temp_file);
        writer.write_frame(original_frame);
        writer.close();
        
        // Read back
        XYZReader reader(temp_file);
        auto read_frame = reader.read_frame(0);
        
        // Verify roundtrip
        CHECK(read_frame.get_metadata<std::string>("comment") == "Roundtrip test");
        
        const auto& read_atoms = read_frame["atoms"];
        const auto& read_coords = read_atoms.get<xt::xarray<float>>("coordinates");
        const auto& read_atomic_nums = read_atoms.get<xt::xarray<float>>("atomic_numbers");
        
        CHECK(read_coords.shape()[0] == 2);
        CHECK(approx_equal(read_coords(1, 0), 1.234));
        CHECK(approx_equal(read_coords(1, 1), -2.567));
        CHECK(approx_equal(read_coords(1, 2), 3.890));
        CHECK(read_atomic_nums(0) == 8.0);
        CHECK(read_atomic_nums(1) == 1.0);
        
        std::filesystem::remove(temp_file);
    }
}