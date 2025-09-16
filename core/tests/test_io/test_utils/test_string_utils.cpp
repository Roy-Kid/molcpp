#include <catch2/catch_test_macros.hpp>
#include <molcpp/io/utils/string_utils.hpp>

using namespace molcpp::io::utils::string;

TEST_CASE("String utilities functionality", "[io][utils][string]") {
    
    SECTION("Trim whitespace") {
        CHECK(trim("  hello world  ") == "hello world");
        CHECK(trim("\t\n  test  \r\n") == "test");
        CHECK(trim("") == "");
        CHECK(trim("   ") == "");
        CHECK(trim("no_spaces") == "no_spaces");
    }
    
    SECTION("Case conversion") {
        CHECK(to_lower("HELLO World") == "hello world");
        CHECK(to_lower("") == "");
        CHECK(to_lower("123ABC") == "123abc");
        
        CHECK(to_upper("hello World") == "HELLO WORLD");
        CHECK(to_upper("") == "");
        CHECK(to_upper("123abc") == "123ABC");
    }
    
    SECTION("String splitting") {
        auto result = split("a,b,c,d", ',');
        CHECK(result.size() == 4);
        CHECK(result[0] == "a");
        CHECK(result[3] == "d");
        
        auto whitespace_result = split_whitespace("  hello   world  test  ");
        CHECK(whitespace_result.size() == 3);
        CHECK(whitespace_result[0] == "hello");
        CHECK(whitespace_result[1] == "world");
        CHECK(whitespace_result[2] == "test");
        
        // Test empty string
        auto empty_result = split("", ',');
        CHECK(empty_result.size() == 1);
        CHECK(empty_result[0] == "");
    }
    
    SECTION("String prefix/suffix checks") {
        CHECK(starts_with("hello world", "hello"));
        CHECK(starts_with("test", "test"));
        CHECK_FALSE(starts_with("hello", "hello world"));
        CHECK_FALSE(starts_with("hello", "hi"));
        
        CHECK(ends_with("hello world", "world"));
        CHECK(ends_with("test", "test"));
        CHECK_FALSE(ends_with("world", "hello world"));
        CHECK_FALSE(ends_with("world", "ld"));
    }
    
    SECTION("String replacement") {
        CHECK(replace_all("hello world hello", "hello", "hi") == "hi world hi");
        CHECK(replace_all("test", "xyz", "abc") == "test");
        CHECK(replace_all("", "a", "b") == "");
        CHECK(replace_all("aaa", "a", "bb") == "bbbbbb");
        CHECK(replace_all("abc", "abc", "") == "");
    }
}