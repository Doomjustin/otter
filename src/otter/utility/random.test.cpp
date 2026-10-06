#include "random.h"

#include <array>
#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("random uniform values are inside requested ranges", "[otter.utility.random]")
{
    otter::random::seed(1234U);

    for (int i = 0; i < 256; ++i) {
        const auto vi = otter::random::uniform(3, 10);
        CHECK(vi >= 3);
        CHECK(vi < 10);

        const auto vf = otter::random::uniform(1.5, 4.5);
        CHECK(vf >= 1.5);
        CHECK(vf < 4.5);
    }
}

TEST_CASE("random supports deterministic replay with same seed", "[otter.utility.random]")
{
    otter::random::seed(42U);
    std::array<int, 16> seq1{};
    for (auto& value : seq1)
        value = otter::random::uniform(0, 1000);

    otter::random::seed(42U);
    std::array<int, 16> seq2{};
    for (auto& value : seq2)
        value = otter::random::uniform(0, 1000);

    CHECK(seq1 == seq2);
}

TEST_CASE("random bernoulli handles probability edges", "[otter.utility.random]")
{
    otter::random::seed(77U);
    for (int i = 0; i < 64; ++i) {
        CHECK(otter::random::bernoulli(0.0) == false);
        CHECK(otter::random::bernoulli(1.0) == true);
    }
}

TEST_CASE("random choice returns an element from range", "[otter.utility.random]")
{
    const std::array<int, 5> data{ 11, 22, 33, 44, 55 };
    otter::random::seed(100U);

    for (int i = 0; i < 64; ++i) {
        const auto value = otter::random::choice(data);
        CHECK(std::ranges::find(data, value) != data.end());
    }
}

TEST_CASE("random shuffle preserves all elements", "[otter.utility.random]")
{
    std::vector<int> values{ 1, 2, 3, 4, 5, 6, 7, 8 };
    const auto original = values;

    otter::random::seed(99U);
    otter::random::shuffle(values);

    CHECK(values.size() == original.size());
    std::ranges::sort(values);
    CHECK(values == original);
}

TEST_CASE("random sample returns requested count from input range", "[otter.utility.random]")
{
    const std::vector<int> values{ 10, 20, 30, 40, 50, 60 };

    otter::random::seed(2025U);
    const auto picked = otter::random::sample(values, 3);

    CHECK(picked.size() == 3);
    for (const auto value : picked)
        CHECK(std::ranges::find(values, value) != values.end());
}
