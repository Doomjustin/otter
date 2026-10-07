#include "map.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("meta::Map supports lookup, contains and typed defaults", "[otter.utility.meta]")
{
    constexpr auto config = otter::meta::Map{
        otter::meta::kv("port", 8080),
        otter::meta::kv("worker_count", 4),
    };

    CHECK(config.size() == 2);
    CHECK(!config.empty());
    CHECK(config.contains("port"));
    CHECK(!config.contains("missing"));
    REQUIRE(config.get("port").has_value());
    CHECK(config.get("port")->get() == 8080);
    CHECK(!config.get("missing").has_value());
    CHECK(config.get_or("worker_count", 1) == 4);
    CHECK(config.get_or("missing", 1) == 1);
    REQUIRE(config["port"].has_value());
    CHECK(config["port"]->get() == 8080);
}

TEST_CASE("meta::Map supports string entries and literal key builder", "[otter.utility.meta]")
{
    constexpr auto labels = otter::meta::Map{
        otter::meta::kv("mode", "debug"),
        otter::meta::kv("role", "worker"),
    };

    REQUIRE(labels.get("mode").has_value());
    CHECK(labels.get("mode")->get() == std::string_view{ "debug" });
    CHECK(labels.get_or("role", "fallback") == std::string_view{ "worker" });
    CHECK(labels.get_or("missing", "fallback") == std::string_view{ "fallback" });
}
