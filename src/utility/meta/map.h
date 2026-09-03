#ifndef OTTER_UTILITY_META_MAP_H
#define OTTER_UTILITY_META_MAP_H

#include <cstddef>
#include <string_view>

#include "string.h"

namespace otter::meta {

template<std::size_t N, typename V>
struct Entry {
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

    constexpr auto get(std::string_view key) const noexcept -> std::optional<Value>
    {
        for (const auto& [k, v] : data_)
            if (k == key)
                return v;

        return {};
    }

    constexpr auto contains(std::string_view key) const noexcept -> bool
    {
        for (const auto& [k, v] : data_)
            if (k == key)
                return true;

        return false;
    }

    template<std::size_t Len>
    constexpr auto get_or(std::string_view key, const char (&default_value)[Len]) const noexcept
        -> std::string_view
    {
        if (auto result = get(key))
            return *result;

        return std::string_view{ default_value, Len - 1 };
    }

    constexpr auto get_or(std::string_view key, Value default_value) const noexcept -> Value
    {
        if (auto result = get(key))
            return *result;

        return default_value;
    }

    constexpr auto size() const noexcept -> std::size_t
    {
        return N;
    }

    constexpr auto empty() const noexcept -> bool
    {
        return N == 0;
    }

    constexpr auto operator[](std::string_view key) const noexcept -> std::optional<Value>
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
struct KeyTag {
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
    return KeyTag<Key>{};
}

} // namespace literals

} // namespace otter::meta

#endif // OTTER_UTILITY_META_MAP_H