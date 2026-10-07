#ifndef OTTER_LEVELDB_CODING_H
#define OTTER_LEVELDB_CODING_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace otter::leveldb {

struct varint {
    varint() = delete;

    template<std::unsigned_integral T>
    static std::size_t length(T value) noexcept
    {
        // 至少需要一个字节
        std::size_t result = 1;
        while (value >= 128) {
            value >>= 7;
            ++result;
        }

        return result;
    }

    template<typename Iterator, std::unsigned_integral T>
        requires std::output_iterator<Iterator, std::byte>
    static Iterator encode(Iterator iter, T value) noexcept
    {
        while (value > mask) {
            // 写入一个字节，最高位为1表示后面还有字节
            auto byte = static_cast<std::byte>((value & mask) | continuation);
            *iter++ = byte;
            value >>= shift_bits;
        }

        *iter++ = static_cast<std::byte>(value); // 最后一个字节，最高位为0
        return iter;
    }

    template<typename Iterator, std::unsigned_integral T>
        requires std::output_iterator<Iterator, char>
    static Iterator encode(Iterator iter, T value) noexcept
    {
        while (value > mask) {
            auto byte = static_cast<char>((value & mask) | continuation);
            *iter++ = byte;
            value >>= shift_bits;
        }

        *iter++ = static_cast<char>(value); // 最后一个字节，最高位为0
        return iter;
    }

    template<std::unsigned_integral T, std::input_iterator Iterator>
        requires std::convertible_to<typename Iterator::value_type, std::byte>
    static auto decode(Iterator iter) noexcept -> std::pair<T, Iterator>
    {
        T value = 0;
        int shift = 0;
        while (true) {
            auto byte_value = std::to_integer<T>(*iter++);
            value |= (byte_value & mask) << shift;

            if (!is_continuation(byte_value))
                break;

            shift += shift_bits;
        }

        return { value, iter };
    }

    template<std::unsigned_integral T, std::input_iterator Iterator>
        requires std::convertible_to<typename Iterator::value_type, char> ||
                 std::is_same_v<Iterator, const char*>
    static auto decode(Iterator iter) noexcept -> std::pair<T, Iterator>
    {
        T value = 0;
        int shift = 0;
        while (true) {
            const auto byte_value = static_cast<std::uint8_t>(*iter++);
            value |= (byte_value & mask) << shift;

            if (!is_continuation(byte_value))
                break;

            shift += shift_bits;
        }

        return { value, iter };
    }

private:
    static constexpr int shift_bits = 7;
    static constexpr auto mask = 0b01111111;
    static constexpr auto continuation = 0b10000000;

    static bool is_continuation(const std::uint8_t byte) noexcept
    {
        return byte & continuation;
    }
};

struct fixed {
    fixed() = delete;

    template<std::unsigned_integral T, std::output_iterator<std::byte> Iterator>
    static Iterator encode(Iterator iter, T value) noexcept
    {
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            *iter++ = static_cast<std::byte>(value & 0xFF);
            value >>= 8;
        }

        return iter;
    }

    template<typename Iterator, std::unsigned_integral T>
        requires std::output_iterator<Iterator, char>
    static Iterator encode(Iterator iter, T value) noexcept
    {
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            *iter++ = static_cast<char>(value & 0xFF);
            value >>= 8;
        }

        return iter;
    }

    template<std::unsigned_integral T, std::input_iterator Iterator>
        requires std::convertible_to<typename Iterator::value_type, std::byte>
    static auto decode(Iterator iter) noexcept -> std::pair<T, Iterator>
    {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i)
            value |= (static_cast<T>(std::to_integer<std::uint8_t>(*iter++)) << (8 * i));

        return { value, iter };
    }

    template<std::unsigned_integral T, std::input_iterator Iterator>
        requires std::convertible_to<typename Iterator::value_type, char> ||
                 std::is_same_v<Iterator, const char*>
    static auto decode(Iterator iter) noexcept -> std::pair<T, Iterator>
    {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i)
            value |= (static_cast<T>(static_cast<std::uint8_t>(*iter++)) << (8 * i));

        return { value, iter };
    }
};

template<std::uint8_t N, std::unsigned_integral T>
struct pack_bytes_t {
    T value;
};

template<std::uint8_t N, std::unsigned_integral T>
constexpr auto pack_bytes(T value) noexcept
{
    return pack_bytes_t<N, T>{ value };
}

template<std::unsigned_integral T, std::unsigned_integral U, std::uint8_t N>
    requires(N > 0 && N < sizeof(T) && N <= sizeof(U))
constexpr T pack(T dest, pack_bytes_t<N, U> src) noexcept
{
    constexpr auto shift = 8 * N;
    constexpr auto mask = (T{ 1 } << shift) - 1;

    return (dest << shift) | (static_cast<T>(src.value) & mask);
}

template<std::unsigned_integral U, std::uint8_t N, std::unsigned_integral T>
    requires(N > 0 && N < sizeof(T) && N <= sizeof(U))
constexpr auto unpack(const T& packed_value) noexcept -> std::pair<U, T>
{
    constexpr auto shift = 8 * N;
    constexpr auto mask = (T{ 1 } << shift) - 1;
    const auto extracted = static_cast<U>(packed_value & mask);
    const auto remaining = packed_value >> shift;

    return { extracted, remaining };
}

} // namespace otter::leveldb

#endif // OTTER_LEVELDB_CODING_H
