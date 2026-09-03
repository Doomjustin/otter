#include "cast.h"

#include <array>
#include <cstring>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

using namespace otter;

TEST_CASE("as_string converts contiguous ranges and byte spans", "[utility][cast]")
{
    std::string text = "hello";
    REQUIRE(std::string{ as_string(text) } == "hello");

    std::array<std::byte, 5> bytes{
        std::byte{ 'h' }, std::byte{ 'e' }, std::byte{ 'l' }, std::byte{ 'l' }, std::byte{ 'o' }
    };

    std::span<const std::byte> const_bytes{ bytes };
    std::span<std::byte> mutable_bytes{ bytes };

    REQUIRE(std::string{ as_string(const_bytes) } == "hello");
    REQUIRE(std::string{ as_string(mutable_bytes) } == "hello");
}

TEST_CASE("numeric_cast parses valid integers and rejects invalid input", "[utility][cast]")
{
    auto value = numeric_cast<int>("42");
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 42);

    auto invalid = numeric_cast<int>("42x");
    REQUIRE(!invalid.has_value());
    REQUIRE(invalid.error() == std::make_error_code(std::errc::invalid_argument));
}

TEST_CASE("string_cast formats floating point values", "[utility][cast]")
{
    auto result = string_cast(12.5);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "12.5");
}

TEST_CASE("to_uppercase and to_lowercase convert ASCII letters", "[utility][cast]")
{
    REQUIRE(to_uppercase("AbC123") == "ABC123");
    REQUIRE(to_lowercase("AbC123") == "abc123");
}
