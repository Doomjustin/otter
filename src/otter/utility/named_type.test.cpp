#include "named_type.h"

#include <cmath>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

using Counter = otter::Numeric<int, "Counter">;
using Ratio = otter::Numeric<double, "Ratio">;
using OrderId = otter::Numeric<std::size_t, "OrderId">;
using UserId = otter::Numeric<std::size_t, "UserId">;
using LegacyId = otter::NamedType<std::size_t, "LegacyId", otter::comparable, otter::hashable>;

using RawInt = otter::NamedType<int, "RawInt">;

template<typename T>
concept supports_addition = requires(T lhs, T rhs) { lhs + rhs; };

template<typename T>
concept supports_stream_output = requires(std::ostream& os, const T& value) { os << value; };

template<typename T>
concept supports_std_hash = requires(const T& value) {
    { std::hash<T>{}(value) } -> std::convertible_to<std::size_t>;
};

template<template<typename> class... Skills>
concept supports_numeric_alias =
    requires { typename otter::Numeric<int, "AliasProbe", Skills...>; };

template<typename T, template<typename> class... Skills>
concept supports_named_type_alias =
    requires { typename otter::NamedType<T, "NamedTypeProbe", Skills...>; };

TEST_CASE("named_type base wrapper preserves strong typing and explicit construction",
          "[otter.utility.named_type]")
{
    static_assert(!std::is_convertible_v<int, RawInt>);
    static_assert(!std::is_convertible_v<RawInt, int>);
    static_assert(!std::is_constructible_v<RawInt, LegacyId>);

    const auto value = RawInt{ 42 };
    CHECK(value.get() == 42);
    CHECK(*value == 42);
}

TEST_CASE("named_type base wrapper allows mutable access through get and operator*",
          "[otter.utility.named_type]")
{
    auto value = RawInt{ 10 };
    value.get() = 15;
    CHECK(*value == 15);

    *value = 21;
    CHECK(value.get() == 21);
}

TEST_CASE("numeric alias provides arithmetic and comparable behavior", "[otter.utility.named_type]")
{
    auto left = Counter{ 10 };
    const auto right = Counter{ 3 };

    SECTION("supports + - * /")
    {
        CHECK((left + right).get() == 13);
        CHECK((left - right).get() == 7);
        CHECK((left * right).get() == 30);
        CHECK((left / right).get() == 3);
    }

    SECTION("supports pre and post increment/decrement")
    {
        CHECK((++left).get() == 11);
        CHECK((left++).get() == 11);
        CHECK(left.get() == 12);
        CHECK((--left).get() == 11);
        CHECK((left--).get() == 11);
        CHECK(left.get() == 10);
    }

    SECTION("supports integer modulo")
    {
        CHECK((Counter{ 10 } % Counter{ 3 }).get() == 1);
    }
}

TEST_CASE("numeric alias supports floating-point modulo", "[otter.utility.named_type]")
{
    const auto result = Ratio{ 7.5 } % Ratio{ 2.0 };
    CHECK(std::abs(result.get() - 1.5) < 1e-12);
}

TEST_CASE("named_type comparable/hashable/printable interoperate", "[otter.utility.named_type]")
{
    const auto first = UserId{ 7 };
    const auto second = UserId{ 8 };

    CHECK(first < second);
    CHECK(first == UserId{ 7 });

    std::unordered_set<UserId> ids;
    ids.insert(first);
    ids.insert(second);
    CHECK(ids.contains(UserId{ 7 }));
    CHECK(ids.contains(UserId{ 8 }));

    std::ostringstream out;
    out << first;
    CHECK(out.str() == "7");
}

TEST_CASE("numeric alias participates in unordered containers via hash",
          "[otter.utility.named_type]")
{
    std::unordered_set<Counter> values;
    values.insert(Counter{ 1 });
    values.insert(Counter{ 2 });

    CHECK(values.contains(Counter{ 1 }));
    CHECK(values.contains(Counter{ 2 }));
}

TEST_CASE("named_type skill composition controls available operations",
          "[otter.utility.named_type]")
{
    static_assert(!supports_addition<RawInt>);
    static_assert(!supports_stream_output<LegacyId>);
    static_assert(supports_stream_output<UserId>);
    static_assert(supports_std_hash<LegacyId>);
    static_assert(supports_std_hash<UserId>);
}

TEST_CASE("numeric alias rejects duplicate skills at compile time", "[otter.utility.named_type]")
{
    static_assert(supports_numeric_alias<>);
    static_assert(!supports_numeric_alias<otter::arithmetic>);
    static_assert(!supports_numeric_alias<otter::comparable>);
    static_assert(!supports_numeric_alias<otter::hashable>);
    static_assert(!supports_numeric_alias<otter::printable>);
}

TEST_CASE("named_type currently accepts only arithmetic value types", "[otter.utility.named_type]")
{
    static_assert(supports_named_type_alias<int>);
    static_assert(supports_named_type_alias<double, otter::comparable>);
    static_assert(!supports_named_type_alias<std::string>);
    static_assert(!supports_named_type_alias<std::string, otter::comparable>);
}
