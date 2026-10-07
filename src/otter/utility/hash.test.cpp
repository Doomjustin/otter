#include "hash.h"

#include <functional>
#include <string>
#include <unordered_map>

#include <catch2/catch_test_macros.hpp>

template<typename Map>
concept supports_find_with_string_view = requires(Map& map, const std::string_view key) {
    map.find(key);
};

TEST_CASE("hash matches the std::hash implementation for string_view", "[otter.utility.hash]")
{
    constexpr std::string_view text{ "hello" };

    CHECK(otter::hash(otter::use_std_hash(text)) == std::hash<std::string_view>{}(text));
}

TEST_CASE("hash uses fnv-1a for string_view payloads", "[otter.utility.hash]")
{
    constexpr std::string_view text{ "hello" };

    CHECK(otter::hash(otter::use_fnv_1a(text)) == 11831194018420276491uz);
    CHECK(otter::hash(otter::use_fnv_1a("otter")) == 11680007987142795529uz);
}

TEST_CASE("string_hasher supports transparent lookup by string_view", "[otter.utility.hash]")
{
    std::unordered_map<std::string, int, otter::string_hasher, std::equal_to<>> map;
    map["hello"] = 42;

    const auto it = map.find(std::string_view{ "hello" });
    CHECK(it != map.end());
    CHECK(it->second == 42);
    CHECK(map["hello"] == 42);
}

TEST_CASE("pmr_string_hasher supports transparent lookup by string_view", "[otter.utility.hash]")
{
    std::pmr::unordered_map<std::string, int, otter::pmr_string_hasher, std::equal_to<>> map;
    map["otter"] = 7;

    const auto it = map.find(std::string_view{ "otter" });
    CHECK(it != map.end());
    CHECK(it->second == 7);
    CHECK(map["otter"] == 7);
}

TEST_CASE("transparent lookup requires both hasher and comparator to be transparent",
          "[otter.utility.hash]")
{
    using non_transparent_map = std::unordered_map<std::string, int, otter::string_hasher>;
    using transparent_map =
        std::unordered_map<std::string, int, otter::string_hasher, std::equal_to<>>;

    CHECK_FALSE(supports_find_with_string_view<non_transparent_map>);
    CHECK(supports_find_with_string_view<transparent_map>);
}
