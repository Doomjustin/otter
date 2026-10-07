#ifndef OTTER_UTILITY_META_MAP_H
#define OTTER_UTILITY_META_MAP_H

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "string.h"

namespace otter::meta {

template<std::size_t N, typename V>
class Entry {
public:
    std::string_view key;
    V value;

    constexpr Entry(const char (&k)[N], V v)
      : key{ k, N - 1 }
      , value{ std::move(v) }
    {}

    constexpr Entry(std::string_view k, V v)
      : key{ k }
      , value{ std::move(v) }
    {}
};

template<std::size_t N, typename V>
Entry(const char (&)[N], V) -> Entry<N, V>;

template<typename Value, std::size_t N>
class Map {
private:
    using entry_type = std::pair<std::string_view, Value>;
    std::array<entry_type, N> data_;

public:
    template<std::size_t... Len>
    constexpr Map(const Entry<Len, Value>&... entries)
      : data_{ entry_type{ entries.key, entries.value }... }
    {}

    constexpr auto get(std::string_view key) const noexcept
        -> std::optional<std::reference_wrapper<const Value>>
    {
        for (const auto& [k, v] : data_)
            if (k == key)
                return std::cref(v);

        return {};
    }

    [[nodiscard]]
    constexpr bool contains(std::string_view key) const noexcept
    {
        for (const auto& [k, v] : data_)
            if (k == key)
                return true;

        return false;
    }

    template<std::size_t Len>
        requires std::convertible_to<Value, std::string_view>
    constexpr auto get_or(std::string_view key, const char (&default_value)[Len]) const noexcept
        -> std::string_view
    {
        if (auto result = get(key))
            return result->get();

        return std::string_view{ default_value, Len - 1 };
    }

    [[nodiscard]]
    constexpr auto get_or(std::string_view key, const Value& default_value) const noexcept
        -> const Value&
    {
        if (auto result = get(key))
            return result->get();

        return std::as_const(default_value);
    }

    [[nodiscard]]
    constexpr std::size_t size() const noexcept
    {
        return N;
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return N == 0;
    }

    constexpr auto operator[](std::string_view key) const noexcept
        -> std::optional<std::reference_wrapper<const Value>>
    {
        return get(key);
    }
};

template<std::size_t... Len, typename V>
Map(const Entry<Len, V>&...) -> Map<V, sizeof...(Len)>;

template<std::size_t N, typename V>
constexpr auto kv(const char (&key)[N], V&& value) -> Entry<N, std::remove_cvref_t<V>>
{
    return Entry{ key, std::forward<V>(value) };
}

template<std::size_t N1, std::size_t N2>
constexpr auto kv(const char (&key)[N1], const char (&value)[N2]) -> Entry<N1, std::string_view>
{
    return Entry{ key, std::string_view{ value, N2 - 1 } };
}

template<String Key>
struct key_t {
    template<typename V>
    constexpr auto operator=(V&& value) const noexcept
    {
        return Entry<Key.capacity, std::remove_cvref_t<V>>{ Key.view(), std::forward<V>(value) };
    }

    template<std::size_t N>
    constexpr auto operator=(const char (&value)[N]) const noexcept
    {
        return Entry<Key.capacity, std::string_view>{ Key.view(),
                                                      std::string_view{ value, N - 1 } };
    }
};

inline namespace literals {

template<String Key>
constexpr auto operator""_key()
{
    return key_t<Key>{};
}

} // namespace literals

} // namespace otter::meta

#endif // OTTER_UTILITY_META_MAP_H
