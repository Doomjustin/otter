#include "lru_cache.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("lru_cache put/get maintains recency order", "[otter.utility.lru_cache]")
{
    otter::LRUCache<int, int> cache{ 3 };

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    REQUIRE(cache.size() == 3);

    const auto first = cache.begin();
    REQUIRE(first != cache.end());
    CHECK(first->first == 3);
    CHECK(first->second == 30);

    const auto value = cache.get(1);
    REQUIRE(value.has_value());
    CHECK(value->get() == 10);

    const auto new_first = cache.begin();
    REQUIRE(new_first != cache.end());
    CHECK(new_first->first == 1);
    CHECK(new_first->second == 10);
}

TEST_CASE("lru_cache evicts least recently used item", "[otter.utility.lru_cache]")
{
    otter::LRUCache<int, int> cache{ 2 };

    cache.put(1, 100);
    cache.put(2, 200);

    REQUIRE(cache.get(1).has_value());

    cache.put(3, 300);

    CHECK(cache.get(2).has_value() == false);
    REQUIRE(cache.get(1).has_value());
    REQUIRE(cache.get(3).has_value());
}

TEST_CASE("lru_cache updating existing key does not grow cache", "[otter.utility.lru_cache]")
{
    otter::LRUCache<int, int> cache{ 2 };

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(1, 99);

    CHECK(cache.size() == 2);

    const auto value = cache.get(1);
    REQUIRE(value.has_value());
    CHECK(*value == 99);

    const auto first = cache.begin();
    REQUIRE(first != cache.end());
    CHECK(first->first == 1);
}

TEST_CASE("lru_cache get returns mutable reference wrapper", "[otter.utility.lru_cache]")
{
    otter::LRUCache<int, int> cache{ 1 };

    cache.put(7, 70);

    auto value = cache.get(7);
    REQUIRE(value.has_value());

    value->get() = 77;

    const auto updated = cache.get(7);
    REQUIRE(updated.has_value());
    CHECK(updated->get() == 77);
}

TEST_CASE("lru_cache clear removes all elements", "[otter.utility.lru_cache]")
{
    otter::LRUCache<int, int> cache{ 2 };

    cache.put(1, 10);
    cache.put(2, 20);

    REQUIRE(cache.size() == 2);

    cache.clear();

    CHECK(cache.size() == 0);
    CHECK(cache.begin() == cache.end());
    CHECK(cache.get(1).has_value() == false);
    CHECK(cache.get(2).has_value() == false);
}
