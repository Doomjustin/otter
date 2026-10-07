#include "dict.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("meta::Dict supports typed keys, contains and defaults", "[otter.utility.meta]")
{
    constexpr auto settings = otter::meta::Dict{
        otter::meta::sym<"port">(8080),
        otter::meta::sym<"enabled">(true),
    };

    CHECK(settings.size() == 2);
    CHECK(settings.contains<"port">());
    CHECK(!settings.contains<"missing">());
    CHECK(settings.key<"port">() == 8080);
    CHECK(settings.key<"enabled">() == true);
    CHECK(settings.get_or<"port">(80) == 8080);
    CHECK(settings.get_or<"missing">(80) == 80);
}

TEST_CASE("meta::Dict supports symbol helper and string defaults", "[otter.utility.meta]")
{
    constexpr auto info = otter::meta::Dict{
        otter::meta::sym<"name">("otter"),
        otter::meta::sym<"role">("worker"),
    };

    const auto name = info.key<"name">();
    const auto role = info.key<"role">();
    CHECK(name == std::string_view{ "otter" });
    CHECK(role == std::string_view{ "worker" });
    CHECK(info.get_or<"missing">("unknown") == std::string_view{ "unknown" });
}
