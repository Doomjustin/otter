#include <ostream>
#include <string>
#include <system_error>

#include <catch2/catch_test_macros.hpp>
#include <utility/format.h>

using namespace otter;

namespace {
struct named_value {
    std::string value;

    auto to_string() const -> std::string
    {
        return value;
    }
};

struct repr_value {
    std::string value;

    auto to_repr() const -> std::string
    {
        return "[" + value + "]";
    }
};

struct stream_value {
    std::string value;
};

auto operator<<(std::ostream& os, const stream_value& v) -> std::ostream&
{
    return os << v.value;
}
} // namespace

TEST_CASE("std::format supports all formatter branches", "[utility][format]")
{
    const std::error_code ec{ EINVAL, std::generic_category() };
    REQUIRE(std::format("{}", ec) == ec.message());

    enum class status : unsigned char { ok, failed };

    REQUIRE(std::format("{}", status::ok) == "ok");
    REQUIRE(std::format("{}", status::failed) == "failed");

    REQUIRE(std::format("{}", named_value{ "alice" }) == "alice");
    REQUIRE(std::format("{}", repr_value{ "bob" }) == "[bob]");
    REQUIRE(std::format("{}", stream_value{ "carol" }) == "carol");
}