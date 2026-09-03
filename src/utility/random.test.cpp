#include "random.h"

#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace otter;

TEST_CASE("random::seed makes output deterministic", "[utility][random]")
{
    random::seed(1234);
    const auto first = random::uniform(10);
    random::seed(1234);
    const auto second = random::uniform(10);

    REQUIRE(first == second);
    REQUIRE(first >= 0);
    REQUIRE(first < 10);
}

TEST_CASE("random::uniform respects integer and floating bounds", "[utility][random]")
{
    random::seed(7);
    for (int i = 0; i < 100; ++i) {
        const auto integer = random::uniform(5, 20);
        REQUIRE(integer >= 5);
        REQUIRE(integer < 20);
    }

    for (int i = 0; i < 100; ++i) {
        const auto floating = random::uniform(1.0, 5.0);
        REQUIRE(floating >= 1.0);
        REQUIRE(floating < 5.0);
    }
}

TEST_CASE("random::bernoulli returns only valid boolean values", "[utility][random]")
{
    REQUIRE(random::bernoulli(0.0) == false);
    REQUIRE(random::bernoulli(1.0) == true);

    for (int i = 0; i < 50; ++i) {
        const auto value = random::bernoulli(0.5);
        REQUIRE((value == false || value == true));
    }
}

TEST_CASE("random::shuffle choice and sample preserve expected semantics", "[utility][random]")
{
    std::vector<int> values{ 1, 2, 3, 4, 5 };
    const auto before = values;

    random::shuffle(values);
    REQUIRE(values.size() == before.size());
    REQUIRE(std::ranges::is_permutation(values, before));

    const auto picked = random::choice(values);
    REQUIRE(std::ranges::find(values, picked) != values.end());

    const auto sample = random::sample(values, 3);
    REQUIRE(sample.size() == 3);

    const bool is_from_prefix =
        std::is_permutation(sample.begin(), sample.end(), values.begin(), values.begin() + 3);
    const bool is_from_full_collection = std::ranges::is_permutation(sample, values);

    REQUIRE((is_from_prefix || is_from_full_collection));
}
