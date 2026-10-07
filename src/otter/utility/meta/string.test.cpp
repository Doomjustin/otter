#include "string.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("meta::String stores compile-time text and exposes string_view", "[otter.utility.meta]")
{
    constexpr otter::meta::String<6> text{ "otter" };

    static_assert(text.capacity == 6);
    static_assert(text.view() == "otter");

    CHECK(text.view() == "otter");
    CHECK(text.data[5] == '\0');
}

TEST_CASE("meta::String supports comparison via defaulted spaceship", "[otter.utility.meta]")
{
    constexpr otter::meta::String<4> abc{ "abc" };
    constexpr otter::meta::String<4> abd{ "abd" };
    constexpr otter::meta::String<4> abc_copy{ "abc" };

    CHECK(abc == abc_copy);
    CHECK(abc < abd);
}
