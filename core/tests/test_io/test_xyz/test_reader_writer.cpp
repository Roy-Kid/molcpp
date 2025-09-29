#include <catch2/catch_test_macros.hpp>
#include <molcpp/io/xyz.hpp>
#include <molcpp/core/frame.hpp>
#include <filesystem>
#include <fstream>
#include <cmath>

using namespace molcpp;

// Helper to create temporary test file
static std::filesystem::path create_temp_xyz_file(const std::string& content) {
    auto temp_file = std::filesystem::temp_directory_path() / "test_xyz_temp.xyz";
    std::ofstream file(temp_file);
    file << content;
    return temp_file;
}

TEST_CASE("XYZ single frame read", "[io][xyz][reader]") {
    std::string xyz_content =
        "2\n"
        "Water molecule\n"
        "O  0.000  0.000  0.000\n"
        "H  0.757  0.587  0.000\n";
    auto temp_file = create_temp_xyz_file(xyz_content);
    XYZTrajectoryReader reader(temp_file.string());
    CHECK(reader.n_frames() == 1);
    auto frame = reader.read_frame(0);
    CHECK(frame.get_metadata<std::string>("comment") == "Water molecule");
    const Block& atoms = frame["atoms"]; // positions (double) + species (string)
    REQUIRE(atoms.contains("positions"));
    const auto& positions = atoms.get<xt::xarray<double>>("positions");
    CHECK(positions.shape(0) == 2);
    std::filesystem::remove(temp_file);
}

TEST_CASE("XYZ multi-frame read helpers", "[io][xyz][reader]") {
    std::string xyz_content =
        "2\nFrame 1\nC 0 0 0\nC 1 0 0\n"
        "2\nFrame 2\nC 0 0 0\nC 1.1 0 0\n";
    auto temp_file = create_temp_xyz_file(xyz_content);
    XYZTrajectoryReader reader(temp_file.string());
    CHECK(reader.n_frames() == 2);
    auto frame1 = reader.read_frame(0);
    auto frame2 = reader.read_frame(1);
    CHECK(frame1.get_metadata<std::string>("comment") == "Frame 1");
    CHECK(frame2.get_metadata<std::string>("comment") == "Frame 2");
    const auto& atoms2 = frame2["atoms"];
    const auto& pos2 = atoms2.get<xt::xarray<double>>("positions");
    CHECK(pos2(1,0) == Approx(1.1));
    std::filesystem::remove(temp_file);
}

TEST_CASE("XYZ writer roundtrip", "[io][xyz][writer]") {
    auto temp_file = std::filesystem::temp_directory_path() / "test_roundtrip.xyz";
    Frame frame; Block atoms;
    xt::xarray<double> positions = {{0.0,0.0,0.0},{1.234,-2.567,3.890}};
    xt::xarray<std::string> species = {"O","H"};
    atoms.set("positions", positions);
    atoms.set("species", species);
    frame.set_block("atoms", atoms);
    frame.set_metadata("comment", "Roundtrip test");
    auto wr = XYZFrameWriter::write(temp_file.string(), frame);
    REQUIRE(wr.has_value());
    XYZTrajectoryReader reader(temp_file.string());
    auto read_frame = reader.read_frame(0);
    CHECK(read_frame.get_metadata<std::string>("comment") == "Roundtrip test");
    const auto& atoms_r = read_frame["atoms"];
    const auto& pos_r = atoms_r.get<xt::xarray<double>>("positions");
    CHECK(pos_r(1,0) == Approx(1.234));
    CHECK(pos_r(1,1) == Approx(-2.567));
    CHECK(pos_r(1,2) == Approx(3.890));
    std::filesystem::remove(temp_file);
}
