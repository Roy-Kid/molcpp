#include <catch2/catch_test_macros.hpp>
#include <molcpp/io/formats/xyz/parser.hpp>
#include <molcpp/core/frame.hpp>
#include <string>
#include <vector>

using namespace molcpp::io;

TEST_CASE("XYZ Parser functionality", "[io][formats][xyz][parser]") {
    
    SECTION("Parse simple XYZ frame") {
        std::vector<std::string> lines = {
            "2",
            "Test molecule",
            "C  0.000  0.000  0.000",
            "C  1.000  0.000  0.000"
        };
        
        auto frame = xyz_parser::parse_xyz_frame(lines);
        
        CHECK(frame.get_metadata<std::string>("comment") == "Test molecule");
        CHECK(frame.contains_block("atoms"));
        
        const auto& atoms = frame["atoms"];
        CHECK(atoms.contains("coordinates"));
        CHECK(atoms.contains("atomic_numbers"));
        
        const auto& coords = atoms.get<xt::xarray<Real>>("coordinates");
        CHECK(coords.shape()[0] == 2);
        CHECK(coords.shape()[1] == 3);
        CHECK(coords(0, 0) == 0.0);
        CHECK(coords(1, 0) == 1.0);
    }
    
    SECTION("Parse atom line") {
        std::string line = "C  1.234  -2.567  3.890";
        auto atom_data = xyz_parser::parse_atom_line(line);
        
        CHECK(atom_data.element == "C");
        CHECK(atom_data.coordinates[0] == 1.234);
        CHECK(atom_data.coordinates[1] == -2.567);
        CHECK(atom_data.coordinates[2] == 3.890);
    }
    
    SECTION("Parse header") {
        auto [n_atoms, comment] = xyz_parser::parse_header("5", "Water molecule");
        CHECK(n_atoms == 5);
        CHECK(comment == "Water molecule");
    }
    
    SECTION("Validate XYZ frame format") {
        std::vector<std::string> valid_lines = {
            "2",
            "Comment",
            "C  0.0  0.0  0.0",
            "H  1.0  0.0  0.0"
        };
        CHECK(xyz_parser::validate_xyz_frame(valid_lines));
        
        std::vector<std::string> invalid_lines = {
            "2",
            "Comment",
            "C  0.0  0.0  0.0"  // Missing one atom line
        };
        CHECK_FALSE(xyz_parser::validate_xyz_frame(invalid_lines));
    }
    
    SECTION("Parse extended XYZ with properties") {
        std::vector<std::string> lines = {
            "2",
            "Properties=species:S:1:pos:R:3 Lattice=\"10.0 0.0 0.0 0.0 10.0 0.0 0.0 0.0 10.0\"",
            "C  0.000  0.000  0.000",
            "C  1.000  0.000  0.000"
        };
        
        auto frame = xyz_parser::parse_xyz_frame(lines);
        CHECK(frame.contains_block("cell"));
        
        const auto& cell = frame["cell"];
        CHECK(cell.contains("matrix"));
        
        const auto& matrix = cell.get<xt::xarray<Real>>("matrix");
        CHECK(matrix(0, 0) == 10.0);
        CHECK(matrix(1, 1) == 10.0);
        CHECK(matrix(2, 2) == 10.0);
    }
    
    SECTION("Error handling for invalid input") {
        // Test empty input
        std::vector<std::string> empty_lines;
        CHECK_THROWS_AS(xyz_parser::parse_xyz_frame(empty_lines), std::runtime_error);
        
        // Test invalid atom count
        std::vector<std::string> invalid_count = {"not_a_number", "comment"};
        CHECK_THROWS_AS(xyz_parser::parse_xyz_frame(invalid_count), std::runtime_error);
        
        // Test mismatched atom count
        std::vector<std::string> mismatched = {
            "3",
            "Comment",
            "C  0.0  0.0  0.0",
            "H  1.0  0.0  0.0"  // Only 2 atoms, but header says 3
        };
        CHECK_THROWS_AS(xyz_parser::parse_xyz_frame(mismatched), std::runtime_error);
    }
}