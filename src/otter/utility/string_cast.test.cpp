#include "string_cast.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("to_uppercase converts ASCII letters", "[otter.utility.cast]")
{
    const std::locale locale{ "C.UTF-8" };

    CHECK(otter::to_uppercase("hello world", locale) == "HELLO WORLD");
    CHECK(otter::to_uppercase("MiXeD123", locale) == "MIXED123");
    CHECK(otter::to_uppercase("", locale) == "");
}

TEST_CASE("to_lowercase converts ASCII letters", "[otter.utility.cast]")
{
    const std::locale locale{ "C.UTF-8" };

    CHECK(otter::to_lowercase("HELLO WORLD", locale) == "hello world");
    CHECK(otter::to_lowercase("MiXeD123", locale) == "mixed123");
    CHECK(otter::to_lowercase("", locale) == "");
}

TEST_CASE("to_uppercase and to_lowercase support wide strings", "[otter.utility.cast]")
{
    const std::locale locale{ "C.UTF-8" };

    CHECK(otter::to_uppercase(std::wstring_view{ L"hello world" }, locale) == L"HELLO WORLD");
    CHECK(otter::to_lowercase(std::wstring_view{ L"HELLO WORLD" }, locale) == L"hello world");
}

TEST_CASE("trim removes leading and trailing whitespace", "[otter.utility.cast]")
{
    CHECK(otter::trim("  hello world  ") == "hello world");
    CHECK(otter::trim("\t  hello world \n") == "hello world");
    CHECK(otter::trim("   ") == "");
    CHECK(otter::trim("") == "");
}

TEST_CASE("ltrim removes leading whitespace only", "[otter.utility.cast]")
{
    CHECK(otter::ltrim("  hello world  ") == "hello world  ");
    CHECK(otter::ltrim("\t  hello world \n") == "hello world \n");
    CHECK(otter::ltrim("   ") == "");
    CHECK(otter::ltrim("") == "");
}

TEST_CASE("rtrim removes trailing whitespace only", "[otter.utility.cast]")
{
    CHECK(otter::rtrim("  hello world  ") == "  hello world");
    CHECK(otter::rtrim("\t  hello world \n") == "\t  hello world");
    CHECK(otter::rtrim("   ") == "");
    CHECK(otter::rtrim("") == "");
}

TEST_CASE("escape_string escapes common control and quote characters", "[otter.utility.cast]")
{
    const std::string input = "\n\t\r\b\f\v\a\\\"\'";
    CHECK(otter::to_repr(std::string_view{ input }) == "\\n\\t\\r\\b\\f\\v\\a\\\\\\\"\\'");
}

TEST_CASE("escape_string escapes non-printable bytes as hex", "[otter.utility.cast]")
{
    const std::string input{
        static_cast<char>(0x01), 'A', static_cast<char>(0x1F), static_cast<char>(0x7F)
    };
    CHECK(otter::to_repr(std::string_view{ input }) == "\\x01A\\x1F\\x7F");
}

TEST_CASE("escape_string supports wchar input", "[otter.utility.cast]")
{
    const std::wstring input = L"line1\n\"line2\"\\";
    CHECK(otter::to_repr(std::wstring_view{ input }) == L"line1\\n\\\"line2\\\"\\\\");
}
