#include "errors.h"

#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using otter::leveldb::ErrorCode;

TEST_CASE("std::error_code can be returned from ErrorCode and compare directly with ErrorCode", "[otter.leveldb.errors]")
{
    static_assert(std::is_error_code_enum_v<ErrorCode>);

    SECTION("returning ErrorCode from std::error_code function keeps category and value")
    {
        const auto make_error = []() -> std::error_code { return ErrorCode::NotFound; };
        const std::error_code ec = make_error();

        CHECK(ec.category().name() == std::string_view{ "otter.leveldb_error" });
        CHECK(ec.value() == std::to_underlying(ErrorCode::NotFound));
        CHECK(ec.message() == "Not found");
    }

    SECTION("std::error_code compares equal to ErrorCode")
    {
        const std::error_code ec = ErrorCode::Corruption;
        CHECK(ec == ErrorCode::Corruption);
    }

    SECTION("ErrorCode compares equal to std::error_code")
    {
        const std::error_code ec = ErrorCode::Corruption;
        CHECK(ErrorCode::Corruption == ec);
    }
}

TEST_CASE("unknown leveldb error code falls back to unknown message", "[otter.leveldb.errors]")
{
    const auto& category = otter::leveldb::make_error_code(ErrorCode::NotFound).category();
    const std::error_code ec{ 999, category };

    CHECK(ec.message() == "Unknown error");
    CHECK(ec.category() == category);
}
