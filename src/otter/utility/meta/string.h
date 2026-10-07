#ifndef OTTER_UTILITY_META_STRING_H
#define OTTER_UTILITY_META_STRING_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace otter::meta {

/// @brief 编译期固定容量字符串，适合用作非类型模板参数或轻量字面量包装。
///  由于 N 包含结尾的空字符，所以设置时需要确保字符串字面量的长度为 N - 1。
/// @code
/// constexpr otter::meta::String<4> text{ "xin" };
/// static_assert(text.capacity == 4);
/// static_assert(text.view() == "xin");
/// @endcode
///
template<std::size_t N>
class String {
public:
    std::array<char, N> data;

    static constexpr std::size_t capacity{ N };

    constexpr String(const char (&s)[N])
    {
        std::copy_n(s, N, data.data());
    }

    auto operator<=>(const String& other) const = default;

    [[nodiscard]]
    constexpr std::string_view view() const noexcept
    {
        return { data.data(), N - 1 };
    }
};

} // namespace otter::meta

#endif // OTTER_UTILITY_META_STRING_H
