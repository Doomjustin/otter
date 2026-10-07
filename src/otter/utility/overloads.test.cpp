#include "overloads.h"

#include <string>
#include <variant>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("overload dispatches a variant through the matching lambda", "[otter.utility.overload]")
{
    auto visitor = otter::overloads{
        [](int value) { return value + 10; },
        [](double value) { return static_cast<int>(value * 10.0); },
        [](const std::string& text) { return static_cast<int>(text.size()); },
    };

    std::variant<int, double, std::string> value{ "hello" };

    CHECK(std::visit(visitor, value) == 5);
    CHECK(visitor.dance(7) == 17);
    CHECK(otter::dance(visitor, 3.5) == 35);
}

TEST_CASE("overload dance supports const visitors", "[otter.utility.overload]")
{
    const auto visitor = otter::overloads{
        [](int value) { return value * 2; },
        [](double value) { return static_cast<int>(value); },
        [](const std::string& text) { return static_cast<int>(text.front()); },
    };

    CHECK(otter::dance(visitor, 9) == 18);
    CHECK(otter::dance(visitor, 8.75) == 8);
    CHECK(otter::dance(visitor, std::string{ "AB" }) == static_cast<int>('A'));
}
