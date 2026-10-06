#ifndef OTTER_UTILITY_STRING_CAST_H
#define OTTER_UTILITY_STRING_CAST_H

#include <cctype>
#include <locale>
#include <string>
#include <string_view>
#include <type_traits>

namespace otter {

template<typename CharT>
auto to_uppercase(std::basic_string_view<CharT> input, const std::locale& loc = std::locale{})
    -> std::basic_string<CharT>
{
    std::basic_string<CharT> result{ input.begin(), input.end() };
    const auto& facet = std::use_facet<std::ctype<CharT>>(loc);

    for (CharT& ch : result)
        ch = facet.toupper(ch);

    return result;
}

template<typename CharT>
auto to_uppercase(const CharT* input, const std::locale& loc = std::locale{})
    -> std::basic_string<CharT>
{
    return to_uppercase(std::basic_string_view<CharT>{ input }, loc);
}

template<typename CharT>
auto to_lowercase(std::basic_string_view<CharT> input, const std::locale& loc = std::locale{})
    -> std::basic_string<CharT>
{
    std::basic_string<CharT> result{ input.begin(), input.end() };
    const auto& facet = std::use_facet<std::ctype<CharT>>(loc);

    for (CharT& ch : result)
        ch = facet.tolower(ch);

    return result;
}

template<typename CharT>
auto to_lowercase(const CharT* input, const std::locale& loc = std::locale{})
    -> std::basic_string<CharT>
{
    return to_lowercase(std::basic_string_view<CharT>{ input }, loc);
}

template<typename CharT>
auto trim(std::basic_string_view<CharT> input) -> std::basic_string<CharT>
{
    auto start = input.begin();
    auto end = input.end();

    while (start != end && std::isspace(*start))
        ++start;

    while (end != start && std::isspace(*(end - 1)))
        --end;

    return std::basic_string<CharT>{ start, end };
}

template<typename CharT>
auto trim(const CharT* input) -> std::basic_string<CharT>
{
    return trim(std::basic_string_view<CharT>{ input });
}

template<typename CharT>
auto ltrim(std::basic_string_view<CharT> input) -> std::basic_string<CharT>
{
    auto start = input.begin();
    auto end = input.end();

    while (start != end && std::isspace(*start))
        ++start;

    return std::basic_string<CharT>{ start, end };
}

template<typename CharT>
auto ltrim(const CharT* input) -> std::basic_string<CharT>
{
    return ltrim(std::basic_string_view<CharT>{ input });
}

template<typename CharT>
auto rtrim(std::basic_string_view<CharT> input) -> std::basic_string<CharT>
{
    auto start = input.begin();
    auto end = input.end();

    while (end != start && std::isspace(*(end - 1)))
        --end;

    return std::basic_string<CharT>{ start, end };
}

template<typename CharT>
auto rtrim(const CharT* input) -> std::basic_string<CharT>
{
    return rtrim(std::basic_string_view<CharT>{ input });
}

template<typename CharT>
auto to_repr(std::basic_string_view<CharT> input) -> std::basic_string<CharT>
{
    std::basic_string<CharT> result;
    result.reserve(input.size());

    constexpr CharT backslash = static_cast<CharT>('\\');
    constexpr CharT quote = static_cast<CharT>('\"');
    constexpr CharT single_quote = static_cast<CharT>('\'');
    constexpr CharT zero = static_cast<CharT>('0');
    constexpr CharT x = static_cast<CharT>('x');
    constexpr CharT n = static_cast<CharT>('n');
    constexpr CharT t = static_cast<CharT>('t');
    constexpr CharT r = static_cast<CharT>('r');
    constexpr CharT b = static_cast<CharT>('b');
    constexpr CharT f = static_cast<CharT>('f');
    constexpr CharT v = static_cast<CharT>('v');
    constexpr CharT a = static_cast<CharT>('a');

    constexpr auto hex_digit = [](unsigned int value) -> CharT {
        return static_cast<CharT>((value < 10U) ? ('0' + value) : ('A' + (value - 10U)));
    };

    for (CharT ch : input) {
        using UnsignedCharT = std::make_unsigned_t<CharT>;
        const auto code = static_cast<UnsignedCharT>(ch);

        switch (ch) {
        case static_cast<CharT>('\n'):
            result += backslash;
            result += n;
            break;
        case static_cast<CharT>('\t'):
            result += backslash;
            result += t;
            break;
        case static_cast<CharT>('\r'):
            result += backslash;
            result += r;
            break;
        case static_cast<CharT>('\b'):
            result += backslash;
            result += b;
            break;
        case static_cast<CharT>('\f'):
            result += backslash;
            result += f;
            break;
        case static_cast<CharT>('\v'):
            result += backslash;
            result += v;
            break;
        case static_cast<CharT>('\a'):
            result += backslash;
            result += a;
            break;
        case static_cast<CharT>('\\'):
            result += backslash;
            result += backslash;
            break;
        case static_cast<CharT>('"'):
            result += backslash;
            result += quote;
            break;
        case static_cast<CharT>('\''):
            result += backslash;
            result += single_quote;
            break;
        default:
            if (code < static_cast<UnsignedCharT>(0x20) ||
                code == static_cast<UnsignedCharT>(0x7F)) {
                result += backslash;
                result += x;
                result += hex_digit((code >> 4) & static_cast<UnsignedCharT>(0x0F));
                result += hex_digit(code & static_cast<UnsignedCharT>(0x0F));
            }
            else {
                result += ch;
            }

            break;
        }
    }

    return result;
}

template<typename CharT>
auto to_repr(const CharT* input) -> std::basic_string<CharT>
{
    return to_repr(std::basic_string_view<CharT>{ input });
}

} // namespace otter

#endif // OTTER_UTILITY_STRING_CAST_H
