#ifndef OTTER_UTILITY_META_STRING_H
#define OTTER_UTILITY_META_STRING_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace otter::meta {

template<std::size_t N>
struct String {
    std::array<char, N> data;

    static constexpr std::size_t capacity{ N };

    constexpr String(const char (&s)[N])
    {
        std::copy_n(s, N, data.data());
    }

    auto operator<=>(const String& other) const = default;

    [[nodiscard]] constexpr auto view() const noexcept -> std::string_view
    {
        return { data.data(), N - 1 };
    }
};

} // namespace otter::meta

#endif // OTTER_UTILITY_META_STRING_H