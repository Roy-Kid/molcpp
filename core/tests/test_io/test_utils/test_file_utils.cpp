#include <catch2/catch_test_macros.hpp>
#include <molcpp/io/utils/file_utils.hpp>
#include <filesystem>
#include <fstream>

using namespace molcpp::io::utils::file;

TEST_CASE("File utilities functionality", "[io][utils][file]") {
    
    SECTION("File existence checks") {
        // Create a temporary file
        auto temp_file = std::filesystem::temp_directory_path() / "test_exists.txt";
        std::ofstream(temp_file) << "test content";
        
        CHECK(exists(temp_file));
        CHECK(is_regular_file(temp_file));
        CHECK_FALSE(is_directory(temp_file));
        
        std::filesystem::remove(temp_file);
        CHECK_FALSE(exists(temp_file));
    }
    
    SECTION("Directory operations") {
        auto temp_dir = std::filesystem::temp_directory_path() / "test_dir";
        
        CHECK(create_directories(temp_dir));
        CHECK(exists(temp_dir));
        CHECK(is_directory(temp_dir));
        CHECK_FALSE(is_regular_file(temp_dir));
        
        std::filesystem::remove(temp_dir);
        CHECK_FALSE(exists(temp_dir));
    }
    
    SECTION("Path operations") {
        std::filesystem::path test_path = "/path/to/file.txt";
        
        CHECK(get_extension(test_path) == ".txt");
        CHECK(get_stem(test_path) == "file");
        CHECK(parent_path(test_path) == "/path/to");
        
        // Test path joining
        std::vector<std::string> components = {"path", "to", "file.xyz"};
        auto joined = join_paths(components);
        CHECK(joined.filename() == "file.xyz");
        CHECK(get_extension(joined) == ".xyz");
    }
    
    SECTION("File size") {
        auto temp_file = std::filesystem::temp_directory_path() / "test_size.txt";
        std::string content = "Hello, World!";
        std::ofstream(temp_file) << content;
        
        auto size = get_size(temp_file);
        CHECK(size == content.length());
        
        std::filesystem::remove(temp_file);
    }
    
    SECTION("Path normalization") {
        // Create a temporary directory structure for testing
        auto temp_dir = std::filesystem::temp_directory_path() / "norm_test";
        auto sub_dir = temp_dir / "sub";
        create_directories(sub_dir);
        
        auto test_file = sub_dir / "test.txt";
        std::ofstream(test_file) << "test";
        
        // Test normalization (this will resolve the absolute path)
        auto normalized = normalize_path(test_file);
        CHECK(normalized.is_absolute());
        CHECK(normalized.filename() == "test.txt");
        
        std::filesystem::remove_all(temp_dir);
    }
    
    SECTION("Error handling") {
        auto non_existent = std::filesystem::path("/non/existent/file.txt");
        CHECK_THROWS_AS(get_size(non_existent), std::runtime_error);
    }
}